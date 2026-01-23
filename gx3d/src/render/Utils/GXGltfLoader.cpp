// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Utils/GXGltfLoader.h>

#include <GenesisX/GX3D/Render/Resources/GXMesh.h>

#include <QFile>
#include <QtEndian>
#include <QVector3D>
#include <QMatrix4x4>
#include <QJsonArray>
#include <QQuaternion>
#include <QJsonObject>
#include <QJsonDocument>

using namespace gx::gx3d::render;

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

static QMatrix4x4 nodeTransform(const QJsonObject& node)
{
    QMatrix4x4 m;
    m.setToIdentity();

    // Prefer explicit matrix
    const auto matA = node.value("matrix").toArray();
    if (matA.size() == 16) {
        float f[16];
        for (int i=0;i<16;++i) f[i] = float(matA.at(i).toDouble());
        // glTF matrices are column-major; QMatrix4x4::constData is column-major too.
        m = QMatrix4x4(f).transposed().transposed(); // keep intent explicit; effectively no-op
        return m;
    }

    // TRS
    QVector3D t(0,0,0);
    auto ta = node.value("translation").toArray();
    if (ta.size()==3) t = QVector3D(float(ta[0].toDouble()), float(ta[1].toDouble()), float(ta[2].toDouble()));

    QVector3D s(1,1,1);
    auto sa = node.value("scale").toArray();
    if (sa.size()==3) s = QVector3D(float(sa[0].toDouble()), float(sa[1].toDouble()), float(sa[2].toDouble()));

    // rotation quaternion (x,y,z,w)
    auto ra = node.value("rotation").toArray();
    float qx=0,qy=0,qz=0,qw=1;
    if (ra.size()==4) { qx=float(ra[0].toDouble()); qy=float(ra[1].toDouble()); qz=float(ra[2].toDouble()); qw=float(ra[3].toDouble()); }

    // Build matrix
    m.translate(t);
    QQuaternion q(qw, qx, qy, qz);
    m.rotate(q);
    m.scale(s);
    return m;
}

GXMesh *GXGltfLoader::loadMesh(const QUrl &source)
{
    // Accept qrc:/..., file:/..., or raw ":/..."
    if (!source.isValid())
        return nullptr;

    if (source.scheme() == "qrc" || source.scheme().isEmpty()) {
        const QString p = source.scheme() == "qrc" ? (":" + source.path()) : source.toString();
        return loadMeshFromFilePath(p);
    }

    if (source.isLocalFile())
        return loadMeshFromFilePath(source.toLocalFile());

    // unsupported
    return nullptr;
}

GXMesh *GXGltfLoader::loadMeshFromFilePath(const QString &path)
{
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
    const auto meshesA = root.value("meshes").toArray();
    if (meshesA.isEmpty()) { qWarning() << "GXGltfLoader: no meshes"; return nullptr; }

    // pick first mesh primitive
    const QJsonObject mesh0 = meshesA.at(0).toObject();
    const auto prims = mesh0.value("primitives").toArray();
    if (prims.isEmpty()) { qWarning() << "GXGltfLoader: no primitives"; return nullptr; }

    const QJsonObject prim0 = prims.at(0).toObject();
    const QJsonObject attrs = prim0.value("attributes").toObject();

    const int posAcc = attrs.value("POSITION").toInt(-1);
    const int nrmAcc = attrs.value("NORMAL").toInt(-1);
    const int idxAcc = prim0.value("indices").toInt(-1);

    if (posAcc < 0 || nrmAcc < 0 || idxAcc < 0) {
        qWarning() << "GXGltfLoader: missing POSITION/NORMAL/indices";
        return nullptr;
    }

    AccessView posV, nrmV, idxV;
    if (!makeAccessView(root, glb.bin, posAcc, posV, &err)) { qWarning() << "pos:" << err; return nullptr; }
    if (!makeAccessView(root, glb.bin, nrmAcc, nrmV, &err)) { qWarning() << "nrm:" << err; return nullptr; }
    if (!makeAccessView(root, glb.bin, idxAcc, idxV, &err)) { qWarning() << "idx:" << err; return nullptr; }

    if (posV.compType != 5126 || posV.ncomp != 3) { qWarning() << "GXGltfLoader: POSITION not float3"; return nullptr; }
    if (nrmV.compType != 5126 || nrmV.ncomp != 3) { qWarning() << "GXGltfLoader: NORMAL not float3"; return nullptr; }
    if (!(idxV.compType == 5123 || idxV.compType == 5125) || idxV.ncomp != 1) {
        qWarning() << "GXGltfLoader: indices not u16/u32 scalar"; return nullptr;
    }

    // Optional: apply first node transform (if present)
    QMatrix4x4 M;
    M.setToIdentity();
    const auto nodesA = root.value("nodes").toArray();
    if (!nodesA.isEmpty()) {
        M = nodeTransform(nodesA.at(0).toObject());
    }
    const QMatrix3x3 n3 = M.normalMatrix();
    QMatrix4x4 n4;
    n4.setToIdentity();

    n4(0,0) = n3(0,0); n4(0,1) = n3(0,1); n4(0,2) = n3(0,2);
    n4(1,0) = n3(1,0); n4(1,1) = n3(1,1); n4(1,2) = n3(1,2);
    n4(2,0) = n3(2,0); n4(2,1) = n3(2,1); n4(2,2) = n3(2,2);

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

        QVector3D P = M.map(QVector3D(px, py, pz));
        QVector3D N = n4.mapVector(QVector3D(nx, ny, nz)).normalized();

        verts[i] = GXMesh::Vertex{ P.x(), P.y(), P.z(), N.x(), N.y(), N.z() };
    }

    QVector<quint16> indices16;
    indices16.reserve(idxV.count);

    if (idxV.compType == 5123) {
        for (int i = 0; i < idxV.count; ++i) {
            const uchar* ip = idxV.base + i * idxV.stride;
            indices16 << readLE<quint16>(ip);
        }
    } else {
        // u32 -> clamp to u16 for now (cube is safe)
        for (int i = 0; i < idxV.count; ++i) {
            const uchar* ip = idxV.base + i * idxV.stride;
            const quint32 v = readLE<quint32>(ip);
            indices16 << quint16(v & 0xFFFFu);
        }
    }

    auto* mesh = new GXMesh();
    mesh->setGeometry(verts, indices16);
    return mesh;
}
