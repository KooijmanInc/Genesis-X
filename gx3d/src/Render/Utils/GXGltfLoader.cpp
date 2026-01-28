// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Utils/GXGltfLoader.h>

#include <GenesisX/GX3D/Render/Resources/GXMesh.h>
#include <GenesisX/GX3D/Render/Nodes/GXModelNode.h>
#include <GenesisX/GX3D/Render/Materials/GXPrincipledMaterial.h>

#include <QFile>
#include <QtEndian>
#include <QVector3D>
#include <QMatrix4x4>
#include <QJsonArray>
#include <QQuaternion>
#include <QJsonObject>
#include <QJsonDocument>

using namespace gx::gx3d::render;

struct GXGltfSceneResult
{
    gx::gx3d::scene::GXNode* root = nullptr;
    QHash<QString, QMatrix4x4> empties;
};

struct GXLoadedMeshes {
    QVector<GXMesh*> meshes;
    QVector<int> meshMaterialIndex;
};

static bool readAllBytes(const QString& path, QByteArray& out)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    out = f.readAll();

    return !out.isEmpty();
}

static quint32 rdU32LE(const uchar* p)
{
    return qFromLittleEndian<quint32>(p);
}

struct GlbChunks {
    QJsonObject json;
    QByteArray bin;
};

static bool parseGlb(const QByteArray& glb, GlbChunks& out, QString* err = nullptr)
{
    if (glb.size() < 12) { if (err) *err = "GLB too small"; return false; }

    const uchar* p = reinterpret_cast<const uchar*>(glb.constData());
    const quint32 magic = rdU32LE(p + 0);
    const quint32 version = rdU32LE(p + 4);
    const quint32 length = rdU32LE(p + 8);

    if (magic != 0x46546C67u) { if (err) *err = "Not a GLB file"; return false; } // 'glTF'
    if (version != 2)         { if (err) *err = "Unsupported glTF version"; return false; }
    if (length != quint32(glb.size())) {
        // Some writers may pad; be tolerant but safe
        if (length > quint32(glb.size())) { if (err) *err = "GLB length mismatch"; return false; }
    }

    int offset = 12;
    QByteArray jsonChunk;
    QByteArray binChunk;

    while (offset + 8 <= glb.size()) {
        const quint32 chunkLen  = rdU32LE(p + offset + 0);
        const quint32 chunkType = rdU32LE(p + offset + 4);
        offset += 8;

        if (offset + int(chunkLen) > glb.size()) { if (err) *err = "Chunk overflow"; return false; }

        const QByteArray chunkData = glb.mid(offset, int(chunkLen));
        offset += int(chunkLen);

        if (chunkType == 0x4E4F534Au) { // JSON
            jsonChunk = chunkData;
        } else if (chunkType == 0x004E4942u) { // BIN
            binChunk = chunkData;
        }
    }

    if (jsonChunk.isEmpty()) { if (err) *err = "Missing JSON chunk"; return false; }

    const auto doc = QJsonDocument::fromJson(jsonChunk);
    if (!doc.isObject()) { if (err) *err = "Invalid JSON"; return false; }

    out.json = doc.object();
    out.bin  = binChunk;

    return true;
}

static int jInt(const QJsonObject& o, const char* k, int def = -1)
{
    const auto v = o.value(QLatin1String(k));
    return v.isDouble() ? v.toInt() : def;
}

static QJsonObject jObjAt(const QJsonArray& a, int idx)
{
    if (idx < 0 || idx >= a.size()) return {};
    return a.at(idx).toObject();
}

static QJsonObject getAccessor(const QJsonObject& root, int accessorIndex)
{
    return jObjAt(root.value("accessors").toArray(), accessorIndex);
}

static QJsonObject getBufferView(const QJsonObject& root, int bufferViewIndex)
{
    return jObjAt(root.value("bufferViews").toArray(), bufferViewIndex);
}

static int componentByteSize(int componentType)
{
    switch (componentType) {
    case 5120: return 1; // BYTE
    case 5121: return 1; // UNSIGNED_BYTE
    case 5122: return 2; // SHORT
    case 5123: return 2; // UNSIGNED_SHORT
    case 5125: return 4; // UNSIGNED_INT
    case 5126: return 4; // FLOAT
    default:   return 0;
    }
}

static int typeCount(const QString& type)
{
    if (type == "SCALAR") return 1;
    if (type == "VEC2")   return 2;
    if (type == "VEC3")   return 3;
    if (type == "VEC4")   return 4;
    if (type == "MAT4")   return 16;
    return 0;
}

template<typename T>
static T readLE(const uchar* p);

template<> float  readLE<float>(const uchar* p)  { quint32 u = rdU32LE(p); float f; memcpy(&f, &u, 4); return f; }
template<> quint16 readLE<quint16>(const uchar* p){ return qFromLittleEndian<quint16>(p); }
template<> quint32 readLE<quint32>(const uchar* p){ return rdU32LE(p); }

struct AccessView {
    const uchar* base = nullptr;
    int count = 0;
    int stride = 0;
    int compType = 0;
    int ncomp = 0;
};

static bool makeAccessView(const QJsonObject& root, const QByteArray& bin, int accessorIndex, AccessView& out, QString* err=nullptr)
{
    const auto acc = getAccessor(root, accessorIndex);
    if (acc.isEmpty()) { if (err) *err = "Missing accessor"; return false; }

    const int bufferViewIndex = jInt(acc, "bufferView", -1);
    if (bufferViewIndex < 0) { if (err) *err = "Accessor has no bufferView"; return false; }

    const auto bv = getBufferView(root, bufferViewIndex);
    if (bv.isEmpty()) { if (err) *err = "Missing bufferView"; return false; }

    const int bvByteOffset = jInt(bv, "byteOffset", 0);
    const int accByteOffset = jInt(acc, "byteOffset", 0);
    const int byteOffset = bvByteOffset + accByteOffset;

    const int count = jInt(acc, "count", 0);
    const int compType = jInt(acc, "componentType", 0);
    const QString type = acc.value("type").toString();

    const int ncomp = typeCount(type);
    const int csz = componentByteSize(compType);
    if (count <= 0 || ncomp <= 0 || csz <= 0) { if (err) *err = "Bad accessor layout"; return false; }

    const int stride = jInt(bv, "byteStride", ncomp * csz);

    if (byteOffset < 0 || byteOffset + stride * count > bin.size()) {
        if (err) *err = "Accessor out of BIN range";
        return false;
    }

    out.base = reinterpret_cast<const uchar*>(bin.constData()) + byteOffset;
    out.count = count;
    out.stride = stride;
    out.compType = compType;
    out.ncomp = ncomp;
    return true;
}

// static QMatrix4x4 nodeTransform(const QJsonObject& node)
// {
//     QMatrix4x4 m;
//     m.setToIdentity();

//     // Prefer explicit matrix
//     const auto matA = node.value("matrix").toArray();
//     if (matA.size() == 16) {
//         float f[16];
//         for (int i=0;i<16;++i) f[i] = float(matA.at(i).toDouble());
//         // glTF matrices are column-major; QMatrix4x4::constData is column-major too.
//         m = QMatrix4x4(f).transposed().transposed(); // keep intent explicit; effectively no-op
//         return m;
//     }

//     // TRS
//     QVector3D t(0,0,0);
//     auto ta = node.value("translation").toArray();
//     if (ta.size()==3) t = QVector3D(float(ta[0].toDouble()), float(ta[1].toDouble()), float(ta[2].toDouble()));

//     QVector3D s(1,1,1);
//     auto sa = node.value("scale").toArray();
//     if (sa.size()==3) s = QVector3D(float(sa[0].toDouble()), float(sa[1].toDouble()), float(sa[2].toDouble()));

//     // rotation quaternion (x,y,z,w)
//     auto ra = node.value("rotation").toArray();
//     float qx=0,qy=0,qz=0,qw=1;
//     if (ra.size()==4) { qx=float(ra[0].toDouble()); qy=float(ra[1].toDouble()); qz=float(ra[2].toDouble()); qw=float(ra[3].toDouble()); }

//     // Build matrix
//     m.translate(t);
//     QQuaternion q(qw, qx, qy, qz);
//     m.rotate(q);
//     m.scale(s);
//     return m;
// }

QVector<GXMesh*> GXGltfLoader::loadMesh(const QUrl &source)
{
    // Accept qrc:/..., file:/..., or raw ":/..."
    if (!source.isValid())
        return {};

    if (source.scheme() == "qrc" || source.scheme().isEmpty()) {
        const QString p = source.scheme() == "qrc" ? (":" + source.path()) : source.toString();
        return loadMeshesFromFilePath(p);
    }

    if (source.isLocalFile())
        return loadMeshesFromFilePath(source.toLocalFile());

    // unsupported
    return {};
}

QVector<GXMesh*> GXGltfLoader::loadMeshesFromFilePath(const QString &path)
{
    QByteArray bytes;
    if (!readAllBytes(path, bytes)) {
        qWarning() << "GXGltfLoader: cannot read" << path;
        return {};
    }

    GlbChunks glb;
    QString err;
    if (!parseGlb(bytes, glb, &err)) {
        qWarning() << "GXGltfLoader:" << err << "path=" << path;
        return {};
    }

    const QJsonObject root = glb.json;
    const auto meshesA = root.value("meshes").toArray();
    if (meshesA.isEmpty()) {
        qWarning() << "GXGltfLoader: no meshes";
        return {};
    }

    // auto requireFloat3 = [&](const AccessView &v, const char *name) -> bool {
    //     if (v.compType != 5126 || v.ncomp != 3) {
    //         qWarning() << "GXGltfLoader:" << name << "not float3";
    //         return false;
    //     }
    //     return true;
    // };

    // auto requireIndexScalar = [&](const AccessView &v) -> bool {
    //     if (!((v.compType == 5123 || v.compType == 5125) && v.ncomp == 1)) {
    //         qWarning() << "GXGltfLoader: indices not u16/u32 scalar";
    //         return false;
    //     }
    //     return true;
    // };

    QVector<GXMesh*> out;
    out.reserve(meshesA.size());

    for (int mi = 0; mi < meshesA.size(); ++mi) {
        const QJsonObject meshObj = meshesA.at(mi).toObject();
        const auto prims = meshObj.value("primitives").toArray();
        if (prims.isEmpty()) {
            qWarning() << "GXGltfLoader: mesh" << mi << "has no primitives";
            continue;
        }

        // --- Use prim0 to get vertex streams (step 1: shared vertex buffers) ---
        const QJsonObject prim0 = prims.at(0).toObject();
        const QJsonObject attrs0 = prim0.value("attributes").toObject();

        const int posAcc = attrs0.value("POSITION").toInt(-1);
        const int nrmAcc = attrs0.value("NORMAL").toInt(-1);
        const int uvAcc  = attrs0.value("TEXCOORD_0").toInt(-1);

        if (posAcc < 0 || nrmAcc < 0) {
            qWarning() << "GXGltfLoader: mesh" << mi << "missing POSITION/NORMAL";
            continue;
        }

        AccessView posV, nrmV, uvV;
        if (!makeAccessView(root, glb.bin, posAcc, posV, &err)) { qWarning() << "pos:" << err; continue; }
        if (!makeAccessView(root, glb.bin, nrmAcc, nrmV, &err)) { qWarning() << "nrm:" << err; continue; }

        // if (!requireFloat3(posV, "POSITION")) continue;
        // if (!requireFloat3(nrmV, "NORMAL")) continue;

        bool hasUv = false;
        if (uvAcc >= 0) {
            if (!makeAccessView(root, glb.bin, uvAcc, uvV, &err)) {
                qWarning() << "uv:" << err << "(continuing without UVs)";
            } else if (uvV.compType != 5126 || uvV.ncomp != 2) {
                qWarning() << "GXGltfLoader: TEXCOORD_0 not float2 (ignore)";
            } else {
                hasUv = true;
            }
        }

        QVector<GXMesh::Vertex> verts;
        verts.resize(posV.count);

        for (int i = 0; i < posV.count; ++i) {
            const uchar* pp = posV.base + i * posV.stride;
            const uchar* np = nrmV.base + i * nrmV.stride;

            const float px = readLE<float>(pp + 0);
            const float py = readLE<float>(pp + 4);
            const float pz = readLE<float>(pp + 8);

            const float nx = readLE<float>(np + 0);
            const float ny = readLE<float>(np + 4);
            const float nz = readLE<float>(np + 8);

            float u = 0.0f, v = 0.0f;
            if (hasUv) {
                const uchar* tp = uvV.base + i * uvV.stride;
                u = readLE<float>(tp + 0);
                v = readLE<float>(tp + 4);
            }

            verts[i] = GXMesh::Vertex{ px, py, pz, nx, ny, nz, u, v };
        }

        // --- Build combined indices + submeshes for ALL primitives ---
        QVector<GXMesh::Vertex> allVerts;
        allVerts.reserve(4096);

        QVector<quint32> allIndices;
        allIndices.reserve(8192);

        auto* mesh = new GXMesh();

        quint32 indexOffset = 0;

        for (int pi = 0; pi < prims.size(); ++pi) {
            const QJsonObject prim = prims.at(pi).toObject();
            const QJsonObject attrs = prim.value("attributes").toObject();

            const int posAcc = attrs.value("POSITION").toInt(-1);
            const int nrmAcc = attrs.value("NORMAL").toInt(-1);
            const int uvAcc  = attrs.value("TEXCOORD_0").toInt(-1);
            const int idxAcc = prim.value("indices").toInt(-1);

            if (posAcc < 0 || nrmAcc < 0 || idxAcc < 0) {
                qWarning() << "GXGltfLoader: mesh" << mi << "prim" << pi << "missing POSITION/NORMAL/indices";
                continue;
            }

            AccessView posV, nrmV, idxV, uvV;
            if (!makeAccessView(root, glb.bin, posAcc, posV, &err)) { qWarning() << "pos:" << err; continue; }
            if (!makeAccessView(root, glb.bin, nrmAcc, nrmV, &err)) { qWarning() << "nrm:" << err; continue; }
            if (!makeAccessView(root, glb.bin, idxAcc, idxV, &err)) { qWarning() << "idx:" << err; continue; }

            bool hasUv = false;
            if (uvAcc >= 0) {
                if (makeAccessView(root, glb.bin, uvAcc, uvV, &err) && uvV.compType == 5126 && uvV.ncomp == 2)
                    hasUv = true;
            }

            if (posV.compType != 5126 || posV.ncomp != 3) { qWarning() << "GXGltfLoader: POSITION not float3"; continue; }
            if (nrmV.compType != 5126 || nrmV.ncomp != 3) { qWarning() << "GXGltfLoader: NORMAL not float3"; continue; }
            if (!((idxV.compType == 5123 || idxV.compType == 5125) && idxV.ncomp == 1)) {
                qWarning() << "GXGltfLoader: indices not u16/u32 scalar"; continue;
            }

            // base vertex for this primitive
            const quint32 baseVertex = quint32(allVerts.size());

            // append vertices for this primitive
            const int vcount = posV.count;
            allVerts.resize(allVerts.size() + vcount);

            for (int i = 0; i < vcount; ++i) {
                const uchar* pp = posV.base + i * posV.stride;
                const uchar* np = nrmV.base + i * nrmV.stride;

                const float px = readLE<float>(pp + 0);
                const float py = readLE<float>(pp + 4);
                const float pz = readLE<float>(pp + 8);

                const float nx = readLE<float>(np + 0);
                const float ny = readLE<float>(np + 4);
                const float nz = readLE<float>(np + 8);

                float u = 0.0f, v = 0.0f;
                if (hasUv) {
                    const uchar* tp = uvV.base + i * uvV.stride;
                    u = readLE<float>(tp + 0);
                    v = readLE<float>(tp + 4);
                }

                allVerts[int(baseVertex) + i] = GXMesh::Vertex{ px, py, pz, nx, ny, nz, u, v };
            }

            // append indices, shifted by baseVertex
            const quint32 firstIndex = indexOffset;
            const quint32 icount = quint32(idxV.count);

            if (idxV.compType == 5123) {
                for (int k = 0; k < idxV.count; ++k) {
                    const uchar* ip = idxV.base + k * idxV.stride;
                    allIndices.append(baseVertex + quint32(readLE<quint16>(ip)));
                }
            } else {
                for (int k = 0; k < idxV.count; ++k) {
                    const uchar* ip = idxV.base + k * idxV.stride;
                    allIndices.append(baseVertex + readLE<quint32>(ip));
                }
            }

            GXSubMesh sm;
            sm.indexOffset  = firstIndex;
            sm.indexCount   = icount;
            sm.materialSlot = prim.value("material").toInt(-1);
            sm.baseVertex   = 0; // because we baked baseVertex into indices already
            mesh->addSubMesh(sm);

            indexOffset += icount;
        }


        // for (int pi = 0; pi < prims.size(); ++pi) {
        //     const QJsonObject prim = prims.at(pi).toObject();
        //     const int idxAcc = prim.value("indices").toInt(-1);
        //     if (idxAcc < 0) {
        //         qWarning() << "GXGltfLoader: mesh" << mi << "prim" << pi << "has no indices";
        //         continue;
        //     }

        //     AccessView idxV;
        //     if (!makeAccessView(root, glb.bin, idxAcc, idxV, &err)) {
        //         qWarning() << "idx:" << err << "mesh" << mi << "prim" << pi;
        //         continue;
        //     }

        //     if (!requireIndexScalar(idxV))
        //         continue;

        //     const int matSlot = prim.value("material").toInt(-1);

        //     const quint32 firstIndex = indexOffset;
        //     const quint32 count = quint32(idxV.count);

        //     if (idxV.compType == 5123) {
        //         for (int k = 0; k < idxV.count; ++k) {
        //             const uchar* ip = idxV.base + k * idxV.stride;
        //             allIndices.append(quint32(readLE<quint16>(ip)));
        //         }
        //     } else { // 5125
        //         for (int k = 0; k < idxV.count; ++k) {
        //             const uchar* ip = idxV.base + k * idxV.stride;
        //             allIndices.append(readLE<quint32>(ip));
        //         }
        //     }

        //     GXSubMesh sm;
        //     sm.indexOffset = firstIndex;
        //     sm.indexCount  = count;
        //     sm.materialSlot = matSlot;
        //     mesh->addSubMesh(sm);

        //     indexOffset += count;
        // }

        if (mesh->subMeshes().isEmpty() || allIndices.isEmpty()) {
            qWarning() << "GXGltfLoader: mesh" << mi << "produced no submeshes/indices";
            delete mesh;
            continue;
        }

        mesh->setGeometry(allVerts, allIndices);
        // qDebug() << "mesh" << mi
        //          << "prims=" << prims.size()
        //          << "verts=" << allVerts.size()
        //          << "indices=" << allIndices.size()
        //          << "subMeshes=" << mesh->subMeshes().size();

        out.push_back(mesh);
    }

    return out;
}


// QVector<GXMesh*> GXGltfLoader::loadMeshesFromFilePath(const QString &path)
// {
//     QByteArray bytes;
//     if (!readAllBytes(path, bytes)) {
//         qWarning() << "GXGltfLoader: cannot read" << path;
//         return {};
//     }

//     GlbChunks glb;
//     QString err;
//     if (!parseGlb(bytes, glb, &err)) {
//         qWarning() << "GXGltfLoader:" << err << "path=" << path;
//         return {};
//     }

//     const QJsonObject root = glb.json;
//     const auto meshesA = root.value("meshes").toArray();
//     if (meshesA.isEmpty()) { qWarning() << "GXGltfLoader: no meshes"; return {}; }

//     QVector<GXMesh*> out;
//     out.reserve(meshesA.size());

//     for (int mi = 0; mi < meshesA.size(); ++mi) {
//         const QJsonObject meshObj = meshesA.at(mi).toObject();
//         const auto prims = meshObj.value("primitives").toArray();
//         if (prims.isEmpty()) { qWarning() << "GXGltfLoader: no primitives"; continue; }

//         const QJsonObject prim0 = prims.at(0).toObject();
//         const int matSlot = prim0.value("material").toInt(-1);
//         qDebug() << "mesh" << mi << "primitive0 material index =" << matSlot;
//         const QJsonObject attrs = prim0.value("attributes").toObject();

//         const int posAcc = attrs.value("POSITION").toInt(-1);
//         const int nrmAcc = attrs.value("NORMAL").toInt(-1);
//         const int idxAcc = prim0.value("indices").toInt(-1);
//         const int uvAcc = attrs.value("TEXCOORD_0").toInt(-1);
//         bool hasUv = false;
//         // quint32 maxIndex = 0;

//         if (posAcc < 0 || nrmAcc < 0 || idxAcc < 0) {
//             qWarning() << "GXGltfLoader: missing POSITION/NORMAL/indices";
//             continue;
//         }

//         AccessView posV, nrmV, idxV, uvV;
//         if (!makeAccessView(root, glb.bin, posAcc, posV, &err)) { qWarning() << "pos:" << err; continue; }
//         if (!makeAccessView(root, glb.bin, nrmAcc, nrmV, &err)) { qWarning() << "nrm:" << err; continue; }
//         if (!makeAccessView(root, glb.bin, idxAcc, idxV, &err)) { qWarning() << "idx:" << err; continue; }
//         if (uvAcc >= 0) {
//             if (!makeAccessView(root, glb.bin, uvAcc, uvV, &err)) {
//                 qWarning() << "uv:" << err << "(continuing without UVs)";
//             } else {
//                 if (uvV.compType != 5126 || uvV.ncomp != 2) {
//                     qWarning() << "GXGltfLoader: TEXCOORD_0 not float2 (ignore)";
//                 } else {
//                     hasUv = true;
//                 }
//             }
//         }


//         if (posV.compType != 5126 || posV.ncomp != 3) { qWarning() << "GXGltfLoader: POSITION not float3"; continue; }
//         if (nrmV.compType != 5126 || nrmV.ncomp != 3) { qWarning() << "GXGltfLoader: NORMAL not float3"; continue; }
//         if (!(idxV.compType == 5123 || idxV.compType == 5125) || idxV.ncomp != 1) {
//             qWarning() << "GXGltfLoader: indices not u16/u32 scalar"; continue;
//         }

//         QVector<GXMesh::Vertex> verts;
//         verts.resize(posV.count);

//         for (int i = 0; i < posV.count; ++i) {
//             const uchar* pp = posV.base + i * posV.stride;
//             const uchar* np = nrmV.base + i * nrmV.stride;

//             const float px = readLE<float>(pp + 0);
//             const float py = readLE<float>(pp + 4);
//             const float pz = readLE<float>(pp + 8);

//             const float nx = readLE<float>(np + 0);
//             const float ny = readLE<float>(np + 4);
//             const float nz = readLE<float>(np + 8);

//             // QVector3D P = M.map(QVector3D(px, py, pz));
//             // QVector3D N = n4.mapVector(QVector3D(nx, ny, nz)).normalized();

//             // float u = 0.0f, v = 0.0f;
//             // if (hasUv) {
//             //     const uchar* tp = uvV.base + i * uvV.stride;
//             //     u = readLE<float>(tp + 0);
//             //     v = readLE<float>(tp + 4);
//             // }
//             const float u = hasUv ? readLE<float>(uvV.base + i * uvV.stride + 0) : 0.0f;
//             const float v = hasUv ? readLE<float>(uvV.base + i * uvV.stride + 4) : 0.0f;

//             verts[i] = GXMesh::Vertex{ px, py, pz, nx, ny, nz, u, v };
//         }

//         QVector<quint16> indices16;
//         indices16.reserve(idxV.count);

//         auto* mesh = new GXMesh();

//         if (idxV.compType == 5123) {
//             QVector<quint16> indices16;
//             indices16.reserve(idxV.count);
//             for (int i = 0; i < idxV.count; ++i) {
//                 const uchar* ip = idxV.base + i * idxV.stride;
//                 indices16 << readLE<quint16>(ip);
//             }
//             mesh->setGeometry(verts, indices16);

//             GXSubMesh sm;
//             sm.indexOffset = 0;
//             sm.indexCount = quint32(idxV.count);   // number of indices
//             sm.materialSlot = prim0.value("material").toInt(-1); // glTF material index (optional)
//             mesh->addSubMesh(sm);
//         } else {
//             QVector<quint32> indices32;
//             indices32.reserve(idxV.count);
//             // u32 -> clamp to u16 for now (cube is safe)
//             for (int i = 0; i < idxV.count; ++i) {
//                 const uchar* ip = idxV.base + i * idxV.stride;
//                 // const quint32 v = readLE<quint32>(ip);
//                 // indices16 << quint16(v & 0xFFFFu);
//                 // maxIndex = qMax(maxIndex, v);
//                 indices32 << readLE<quint32>(ip);
//             }
//             mesh->setGeometry(verts, indices32);

//             GXSubMesh sm;
//             sm.indexOffset = 0;
//             sm.indexCount = quint32(idxV.count);   // number of indices
//             sm.materialSlot = prim0.value("material").toInt(-1); // glTF material index (optional)
//             mesh->addSubMesh(sm);
//         }

//         out.push_back(mesh);
//         continue;
//     }

//     return out;
// }

static void readNodeTRS(const QJsonObject& n, QVector3D& t, QQuaternion& r, QVector3D& s)
{
    // defaults per glTF spec
    t = QVector3D(0, 0, 0);
    r = QQuaternion(1, 0, 0, 0); // w,x,y,z
    s = QVector3D(1, 1, 1);

    // If matrix exists, it overrides TRS
    const auto mA = n.value("matrix").toArray();
    if (mA.size() == 16) {
        QMatrix4x4 M;
        // glTF matrix is column-major
        for (int i = 0; i < 16; ++i)
            M.data()[i] = float(mA.at(i).toDouble());

        // Decompose matrix -> TRS
        t = M.column(3).toVector3D();

        const QVector3D x = M.column(0).toVector3D();
        const QVector3D y = M.column(1).toVector3D();
        const QVector3D z = M.column(2).toVector3D();

        s = QVector3D(x.length(), y.length(), z.length());

        QMatrix3x3 rot;
        if (s.x() != 0) { rot(0,0)=x.x()/s.x(); rot(1,0)=x.y()/s.x(); rot(2,0)=x.z()/s.x(); }
        if (s.y() != 0) { rot(0,1)=y.x()/s.y(); rot(1,1)=y.y()/s.y(); rot(2,1)=y.z()/s.y(); }
        if (s.z() != 0) { rot(0,2)=z.x()/s.z(); rot(1,2)=z.y()/s.z(); rot(2,2)=z.z()/s.z(); }

        r = QQuaternion::fromRotationMatrix(rot);
        return;
    }

    // translation
    const auto tA = n.value("translation").toArray();
    if (tA.size() == 3)
        t = QVector3D(float(tA.at(0).toDouble()), float(tA.at(1).toDouble()), float(tA.at(2).toDouble()));

    // rotation (quat) glTF = [x,y,z,w]
    const auto rA = n.value("rotation").toArray();
    if (rA.size() == 4) {
        const float x = float(rA.at(0).toDouble());
        const float y = float(rA.at(1).toDouble());
        const float z = float(rA.at(2).toDouble());
        const float w = float(rA.at(3).toDouble());
        r = QQuaternion(w, x, y, z);
    }

    // scale
    const auto sA = n.value("scale").toArray();
    if (sA.size() == 3)
        s = QVector3D(float(sA.at(0).toDouble()), float(sA.at(1).toDouble()), float(sA.at(2).toDouble()));
}

static QVector<int> sceneRootNodes(const QJsonObject& root)
{
    QVector<int> out;
    const auto scenesA = root.value("scenes").toArray();
    if (scenesA.isEmpty())
        return out;

    const int sceneIndex = root.value("scene").toInt(0);
    const int si = qBound(0, sceneIndex, scenesA.size() - 1);
    const QJsonObject sceneObj = scenesA.at(si).toObject();

    const auto rootsA = sceneObj.value("nodes").toArray();
    for (const auto& v : rootsA)
        out.push_back(v.toInt(-1));
    return out;
}

static void buildNodeRecursive(
    const QJsonArray& nodesA,
    int nodeIndex,
    const QVector<gx::gx3d::render::GXMesh*>& meshes,
    const QVector<int>& meshToMatIndex,
    const QVector<GXMaterial*>& gltfMaterials,
    GXMaterial* fallbackMat,
    gx::gx3d::scene::GXNode* parent,
    QHash<QString, QVector3D>* tileCenters /* optional, can be nullptr */)
{
    if (nodeIndex < 0 || nodeIndex >= nodesA.size())
        return;

    const QJsonObject n = nodesA.at(nodeIndex).toObject();
    const QString name = n.value("name").toString();

    auto* node = new gx::gx3d::scene::GXNode(parent);
    node->setObjectName(name);

    QVector3D t, s;
    QQuaternion r;
    readNodeTRS(n, t, r, s);

    node->setPosition(t);
    node->setRotation(r);
    node->setScale(s);

    parent->addChild(node);

    // Mesh instance?
    const int meshIndex = n.value("mesh").toInt(-1);

    if (meshIndex >= 0 && meshIndex < meshes.size() && meshes[meshIndex]) {
        auto* model = new gx::gx3d::render::GXModel(node);
        model->setObjectName(name + "_model");
        model->setMesh(meshes[meshIndex]);

        GXMaterial* mat = fallbackMat;
        const int gltfMatIndex = (meshIndex >= 0 && meshIndex < meshToMatIndex.size()) ? meshToMatIndex[meshIndex] : -1;

        if (gltfMatIndex >= 0 && gltfMatIndex < gltfMaterials.size() && gltfMaterials[gltfMatIndex]) mat = gltfMaterials[gltfMatIndex];

        model->clearMaterials();
        for (GXMaterial* gm : gltfMaterials) {
            model->addMaterial(gm);
        }
        model->setMaterial(gltfMaterials.isEmpty() ? mat : gltfMaterials.first());
        // model->materials().append(mat);
        node->addChild(model);
    } else {
        // Empty marker: collect tile centers if desired
        if (tileCenters && name.startsWith("tile_", Qt::CaseInsensitive)) {
            const QString coord = name.mid(5); // "A1"
            tileCenters->insert(coord.toUpper(), t); // local==world if parent identity; later we’ll compute world
        }
    }

    // Recurse children
    const auto childrenA = n.value("children").toArray();
    for (const auto& c : childrenA)
        buildNodeRecursive(nodesA, c.toInt(-1), meshes, meshToMatIndex, gltfMaterials, fallbackMat, node, tileCenters);
}

gx::gx3d::scene::GXNode* GXGltfLoader::loadSceneRoot(const QUrl& source)
{
    // resolve to file path same way as your loadMesh does
    QString path;
    if (!source.isValid())
        return nullptr;

    if (source.scheme() == "qrc" || source.scheme().isEmpty()) {
        path = source.scheme() == "qrc" ? (":" + source.path()) : source.toString();
    } else if (source.isLocalFile()) {
        path = source.toLocalFile();
    } else {
        return nullptr;
    }

    QByteArray bytes;
    if (!readAllBytes(path, bytes)) {
        qWarning() << "GXGltfLoader: cannot read" << path;
        return nullptr;
    }

    GlbChunks glb;
    QString err;
    if (!parseGlb(bytes, glb, &err)) {
        qWarning() << "GXGltfLoader:" << err << "path=" << path;
        return nullptr;
    }

    const QJsonObject root = glb.json;

    const QVector<gx::gx3d::render::GXMesh*> meshes = loadMeshesFromFilePath(path);
    if (meshes.isEmpty()) {
        qWarning() << "GXGltfLoader: no meshes loaded from" << path;
        return nullptr;
    }

    const QJsonArray nodesA = root.value("nodes").toArray();
    if (nodesA.isEmpty()) {
        qWarning() << "GXGltfLoader: no nodes in" << path;
        return nullptr;
    }

    const auto matsA = root.value("materials").toArray();
    // qDebug() << "GLTF materials count =" << matsA.size();
    // for (int i = 0; i < matsA.size(); ++i) {
    //     const QJsonObject mo = matsA.at(i).toObject();
    //     qDebug() << "  mat[" << i << "] name=" << mo.value("name").toString()
    //              << "hasPBR=" << mo.contains("pbrMetallicRoughness")
    //              << "hasEmissive=" << mo.contains("emissiveFactor")
    //              << "hasEmissiveTex=" << mo.contains("emissiveTexture");
    // }

    QVector<GXMaterial*> gltfMaterials;
    gltfMaterials.reserve(matsA.size());

    auto* rootNode = new gx::gx3d::scene::GXNode();
    rootNode->setObjectName("gltfRoot");

    for (int i = 0; i < matsA.size(); ++i) {
        // dev loader: only baseColorFactor (ignore textures for now)
        auto* m = new gx::gx3d::render::GXPrincipledMaterial(rootNode);
        m->setBaseColor("white"); // default

        const QJsonObject mo = matsA.at(i).toObject();
        const QJsonObject pbr = mo.value("pbrMetallicRoughness").toObject();
        const auto bc = pbr.value("baseColorFactor").toArray();
        if (bc.size() >= 3) {
            const float r = float(bc.at(0).toDouble());
            const float g = float(bc.at(1).toDouble());
            const float b = float(bc.at(2).toDouble());
            // if your setBaseColor accepts QColor:
            m->setBaseColor(QColor::fromRgbF(r, g, b, 1.0f));
        }
        const auto emf = mo.value("emissiveFactor").toArray();

        float emissionStrength = 1.0f;
        if (emf.size() >= 3) {
            const float r = float(emf.at(0).toDouble());
            const float g = float(emf.at(1).toDouble());
            const float b = float(emf.at(2).toDouble());
            m->setEmissionColor(QColor::fromRgbF(r, g, b, 1.0f));
            // m->setEmissionStrength(emissionStrength);
        }

        if (mo.contains("extensions")) {
            const auto extensions = mo.value("extensions").toObject();
            if (extensions.contains("KHR_materials_emissive_strength")) {
                const auto emissiveStrength = extensions.value("KHR_materials_emissive_strength").toObject();
                emissionStrength = float(emissiveStrength["emissiveStrength"].toDouble(1.0));
                m->setEmissionStrength(emissionStrength);
            }
        }
        // for (auto& keys : mo.keys()) {
        //     qDebug() << "all keys" << keys;
        // }


        gltfMaterials.push_back(m);
    }

    const auto meshesA = root.value("meshes").toArray();
    QVector<int> meshToMatIndex;
    meshToMatIndex.resize(meshesA.size());
    for (int mi = 0; mi < meshesA.size(); ++mi) {
        meshToMatIndex[mi] = -1;
        const QJsonObject meshObj = meshesA.at(mi).toObject();
        const auto prims = meshObj.value("primitives").toArray();
        if (prims.isEmpty()) continue;
        meshToMatIndex[mi] = prims.at(0).toObject().value("material").toInt(-1);

    }

    auto* fallbackMat = new gx::gx3d::render::GXPrincipledMaterial(rootNode);
    fallbackMat->setBaseColor("purple");

    const auto roots = sceneRootNodes(root);
    if (!roots.isEmpty()) {
        for (int ri : roots)
            buildNodeRecursive(nodesA, ri, meshes, meshToMatIndex, gltfMaterials, fallbackMat, rootNode, nullptr);
    } else {
        buildNodeRecursive(nodesA, 0, meshes, meshToMatIndex, gltfMaterials, fallbackMat, rootNode, nullptr);
    }

    return rootNode;
}

GXGltfSceneResult loadScene(const QUrl& source);
