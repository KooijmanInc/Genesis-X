// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Utils/GXMeshReader.h>

#include <QDebug>
#include <QFile>
#include <QUrl>

namespace gx::gx3d::render {

static constexpr quint32 kMagicGXMS = 0x534D5847u; // 'G''X''M''S' little-endian in u32
static constexpr quint32 kChunkVERT = 0x54524556u; // 'V''E''R''T'
static constexpr quint32 kChunkINDX = 0x58444E49u; // 'I''N''D''X'
static constexpr quint32 kChunkSUBM = 0x4D425553u; // 'S''U''B''M'
static constexpr quint32 kChunkBND0 = 0x30444E42u; // 'B''N''D''0'

// On-disk vertex layout (must match writer DiskVertexV1)
struct DiskVertexV1
{
    float px, py, pz;
    float nx, ny, nz;
    float u0, v0;
};

static inline bool fail(QString* err, const QString& msg)
{
    if (err) *err = msg;
    return false;
}

static inline GXVertex fromDiskVertex(const DiskVertexV1& d)
{
    GXVertex v{};
    v.position = QVector3D(d.px, d.py, d.pz);
    v.normal   = QVector3D(d.nx, d.ny, d.nz);
    v.uv0      = QVector2D(d.u0, d.v0);
    return v;
}

bool GXMeshReader::read(const QString &filePath, GXMeshData &outMesh, QString *errorString, const Options &opt) const
{
    outMesh = GXMeshData{};

    QString path = filePath.trimmed();

    // Accept QML-style URLs like "qrc:/..." or "file:/..."
    QUrl url(path);
    if (url.isValid() && !url.scheme().isEmpty()) {
        if (url.scheme() == "qrc") {
            path = ":" + url.path();   // "/models/..." -> ":/models/..."
        } else if (url.isLocalFile()) {
            path = url.toLocalFile();
        } else {
            if (errorString) *errorString = QString("[GXMeshReader] unsupported URL scheme: %1").arg(url.scheme());
            return false;
        }
    } else {
        // also accept plain strings starting with "qrc:/"
        if (path.startsWith("qrc:/"))
            path = ":" + path.mid(4); // "qrc:/..." -> ":/..."
    }

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return fail(errorString, QString("[GXMeshReader] cannot open %1").arg(filePath));

    QDataStream ds(&f);
    ds.setByteOrder(QDataStream::LittleEndian);
    ds.setFloatingPointPrecision(QDataStream::SinglePrecision);

    auto readU32 = [&](quint32& v) -> bool {
        if (ds.atEnd()) return false;
        ds >> v;
        return ds.status() == QDataStream::Ok;
    };

    auto readF32 = [&](float& v) -> bool {
        if (ds.atEnd()) return false;
        ds >> v;
        return ds.status() == QDataStream::Ok;
    };

    quint32 magic = 0, version = 0;

    if (!readU32(magic) || !readU32(version)) return fail(errorString, QString("[GXMeshReader] file too small (missing header"));

    if (magic != kMagicGXMS) {
        // const quint32 swapped =
        //     (magic >> 24) |
        //     ((magic >> 8) & 0x0000FF00u) |
        //     ((magic << 8) & 0x00FF0000u) |
        //     (magic << 24);

        // if (errorString) {
        //     *errorString = QString("[GXMeshReader] bad magic. got=0x%1 swapped=0x%2 expected=0x%3 (version=%4) path=%5")
        //     .arg(magic,   8, 16, QLatin1Char('0'))
        //         .arg(swapped, 8, 16, QLatin1Char('0'))
        //         .arg(kMagicGXMS, 8, 16, QLatin1Char('0'))
        //         .arg(version)
        //         .arg(path);
        // }
        // return false;
        return fail(errorString, QString("[GXMeshReader] bad magic (not a GXMS .mesh file"));
    }

    if (version != 1u) return fail(errorString, QString("[GXMeshReader] unsupported version %1").arg(version));

    bool haveVERT = false;
    bool haveINDX = false;
    bool haveSUBM = false;

    while (!ds.atEnd()) {
        quint32 chunkId = 0, chunkSize = 0;
        if (!readU32(chunkId)) break;           // allow clean EOF
        if (!readU32(chunkSize))
            return fail(errorString, "GXMeshReader: truncated chunk header");

        // Guard ridiculous sizes
        if (chunkSize > quint32(std::numeric_limits<int>::max()))
            return fail(errorString, "GXMeshReader: chunk too large");

        const qint64 chunkStart = f.pos();
        const qint64 chunkEnd   = chunkStart + qint64(chunkSize);

        if (chunkEnd > f.size())
            return fail(errorString, "GXMeshReader: truncated chunk payload");

        if (chunkId == kChunkVERT) {
            if (chunkSize % quint32(sizeof(DiskVertexV1)) != 0) {
                if (opt.strict)
                    return fail(errorString, "GXMeshReader: VERT chunk size is not multiple of DiskVertexV1");
            }

            const int vCount = int(chunkSize / quint32(sizeof(DiskVertexV1)));
            outMesh.vertices.clear();
            outMesh.vertices.reserve(vCount);

            for (int i = 0; i < vCount; ++i) {
                DiskVertexV1 dv{};
                if (!readF32(dv.px) || !readF32(dv.py) || !readF32(dv.pz) ||
                    !readF32(dv.nx) || !readF32(dv.ny) || !readF32(dv.nz) ||
                    !readF32(dv.u0) || !readF32(dv.v0)) {
                    return fail(errorString, "GXMeshReader: truncated VERT data");
                }
                outMesh.vertices.push_back(fromDiskVertex(dv));
            }
            haveVERT = true;
        }
        else if (chunkId == kChunkINDX) {
            if (chunkSize % 4u != 0u) {
                if (opt.strict)
                    return fail(errorString, "GXMeshReader: INDX chunk size is not multiple of 4");
            }
            const int iCount = int(chunkSize / 4u);
            outMesh.indices.clear();
            outMesh.indices.reserve(iCount);

            for (int i = 0; i < iCount; ++i) {
                quint32 idx = 0;
                if (!readU32(idx))
                    return fail(errorString, "GXMeshReader: truncated INDX data");
                outMesh.indices.push_back(idx);
            }
            haveINDX = true;
        }
        else if (chunkId == kChunkSUBM) {
            if (chunkSize % 12u != 0u) {
                if (opt.strict)
                    return fail(errorString, "GXMeshReader: SUBM chunk size is not multiple of 12");
            }
            const int sCount = int(chunkSize / 12u);
            outMesh.subMeshes.clear();
            outMesh.subMeshes.reserve(sCount);

            for (int i = 0; i < sCount; ++i) {
                GXSubMeshData sm{};
                if (!readU32(sm.firstIndex) ||
                    !readU32(sm.indexCount) ||
                    !readU32(sm.materialIndex)) {
                    return fail(errorString, "GXMeshReader: truncated SUBM data");
                }
                outMesh.subMeshes.push_back(sm);
            }
            haveSUBM = true;
        }
        else if (chunkId == kChunkBND0) {
            // 6 floats
            if (chunkSize != 24u) {
                if (opt.strict)
                    return fail(errorString, "GXMeshReader: BND0 chunk size must be 24 bytes");
            }
            float minx=0, miny=0, minz=0, maxx=0, maxy=0, maxz=0;
            if (!readF32(minx) || !readF32(miny) || !readF32(minz) ||
                !readF32(maxx) || !readF32(maxy) || !readF32(maxz)) {
                return fail(errorString, "GXMeshReader: truncated BND0 data");
            }
            outMesh.boundsMin = QVector3D(minx, miny, minz);
            outMesh.boundsMax = QVector3D(maxx, maxy, maxz);
        }
        else {
            // Unknown chunk: skip it (or fail in strict mode)
            if (opt.strict)
                return fail(errorString, QString("GXMeshReader: unknown chunk 0x%1").arg(chunkId, 8, 16, QLatin1Char('0')));

            f.seek(chunkEnd);
        }

        // Ensure we land exactly at end of chunk even if reader read fewer bytes for non-strict cases
        if (f.pos() != chunkEnd)
            f.seek(chunkEnd);
    }

    if (!haveVERT) return fail(errorString, QString("[GXMeshReader] missing VERT chunk"));
    if (!haveINDX) return fail(errorString, QString("[GXMeshReader] missing INDX chunk"));
    if (!haveSUBM) return fail(errorString, QString("[GXMeshReader] missing SUBM chunk"));

    // Basic sanity
    if (outMesh.vertices.isEmpty()) return fail(errorString, "[GXMeshReader] mesh has no vertices");
    if (outMesh.indices.isEmpty()) return fail(errorString, "[GXMeshReader] mesh has no indices");
    if (opt.requireSubMeshes && outMesh.subMeshes.isEmpty()) return fail(errorString, "[GXMeshReader] mesh has no submeshes");

    return true;
}

bool GXMeshReader::readFile(const QString &filePath, GXMeshData &outMesh, QString *errorString, const Options &opt)
{
    GXMeshReader r;
    return r.read(filePath, outMesh, errorString, opt);
}

}
