// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Utils/GXMeshWriter.h>
#include <GenesisX/utils/io/GXFileWriter.h>

#include <QtGlobal>
#include <QDebug>

#include <cstddef>

namespace gx::gx3d::render {

static constexpr quint32 kMagicGXMS = 0x534D5847u; // "GXMS" (bytes 47 58 4D 53)
static constexpr quint32 kChunkVERT = 0x54524556u; // "VERT"
static constexpr quint32 kChunkINDX = 0x58444E49u; // "INDX"
static constexpr quint32 kChunkSUBM = 0x4D425553u; // "SUBM"
static constexpr quint32 kChunkBND0 = 0x30444E42u; // "BND0"

// Keep the on-disk vertex layout explicit and stable.
// (Even if GXVertex changes later.)
#pragma pack(push, 1)
struct DiskVertexV1
{
    float px, py, pz;
    float nx, ny, nz;
    float u0, v0;
};
#pragma pack(pop)

static DiskVertexV1 toDiskVertex(const GXVertex &v)
{
    DiskVertexV1 d{};
    d.px = float(v.position.x());
    d.py = float(v.position.y());
    d.pz = float(v.position.z());
    d.nx = float(v.normal.x());
    d.ny = float(v.normal.y());
    d.nz = float(v.normal.z());
    d.u0 = float(v.uv0.x());
    d.v0 = float(v.uv0.y());
    return d;
}

static bool validateMesh(const GXMeshData &mesh, QString *err)
{
    // qDebug() << "GXSubMesh offs:"
    //          << offsetof(GXSubMesh, firstIndex)
    //          << offsetof(GXSubMesh, indexCount)
    //          << offsetof(GXSubMesh, materialIndex);
    auto fail = [&](const QString &m) {
        if (err) *err = m;
        return false;
    };

    if (mesh.vertices.isEmpty())
        return fail("GXMeshWriter: mesh has no vertices");
    if (mesh.indices.isEmpty())
        return fail("GXMeshWriter: mesh has no indices");
    if (mesh.subMeshes.isEmpty())
        return fail("GXMeshWriter: mesh has no submeshes");

    // Validate index ranges
    const quint32 vCount = quint32(mesh.vertices.size());
    for (qsizetype i = 0; i < mesh.indices.size(); ++i) {
        const quint32 idx = mesh.indices.at(i);
        if (idx >= vCount)
            return fail(QString("GXMeshWriter: index out of range at %1 (%2 >= %3)")
                            .arg(i).arg(idx).arg(vCount));
    }

    // Validate submesh ranges
    const quint32 iCount = quint32(mesh.indices.size());
    {
        const auto *p = reinterpret_cast<const quint32*>(mesh.subMeshes.constData());
        const int words = mesh.subMeshes.size() * 3;
        // qDebug() << "RAW subMeshes u32 (" << words << "words )";
        QString line;
        for (int i = 0; i < words; ++i) {
            line += QString::number(p[i]);
            line += (i + 1 == words) ? "" : ",";
        }
        // qDebug().noquote() << line;
    }
    for (qsizetype s = 0; s < mesh.subMeshes.size(); ++s) {
        const GXSubMeshData &sm = mesh.subMeshes.at(s);
        if (sm.indexCount == 0) {
            // qDebug() << "validateMesh FAIL: submesh" << s
            //          << "firstIndex=" << sm.firstIndex
            //          << "indexCount=" << sm.indexCount
            //          << "materialIndex=" << sm.materialIndex
            //          << "totalIndices=" << iCount
            //          << "subMeshes=" << mesh.subMeshes.size();
            // also dump all submeshes once when failing:
            for (qsizetype j = 0; j < mesh.subMeshes.size(); ++j) {
                const auto &t = mesh.subMeshes.at(j);
                qDebug() << "  sm" << j << t.firstIndex << t.indexCount << t.materialIndex;
            }
            return fail(QString("GXMeshWriter: submesh %1 has indexCount=0").arg(s));
        }
        if (sm.firstIndex >= iCount)
            return fail(QString("GXMeshWriter: submesh %1 firstIndex out of range").arg(s));
        if (sm.firstIndex + sm.indexCount > iCount)
            return fail(QString("GXMeshWriter: submesh %1 range out of index buffer").arg(s));
    }

    return true;
}

bool GXMeshWriter::write(const GXMeshData &mesh, const QString &filePath, QString *errorString) const
{
    return writeVersioned(mesh, filePath, /*version*/ 1u, errorString);
}

bool GXMeshWriter::writeVersioned(const GXMeshData &mesh, const QString &filePath, quint32 version, QString *errorString) const
{
    if (!validateMesh(mesh, errorString)) return false;
    utils::io::GXFileWriter fw(filePath, utils::io::GXFileWriter::Mode::Binary, utils::io::GXFileWriter::Endian::Little);
    fw.setAtomic(true);

    if (!fw.open()) {
        if (errorString) *errorString = fw.errorString();
        return false;
    }

    // ─────────────────────────────
    // Header
    // ─────────────────────────────
    if (!fw.writeU32(kMagicGXMS) || !fw.writeU32(version)) {
        if (errorString) *errorString = fw.errorString();
        fw.close();
        return false;
    }

    // ─────────────────────────────
    // VERT chunk (DiskVertexV1[])
    // ─────────────────────────────
    {
        const auto cref = fw.beginChunk(kChunkVERT);
        if (!cref.valid()) {
            if (errorString) *errorString = fw.errorString();
            fw.close();
            return false;
        }

        // Stream vertices one by one to avoid a large temporary buffer.
        for (const GXVertex& v : mesh.vertices) {
            const DiskVertexV1 dv = toDiskVertex(v);

            // Use primitive writes so endianness is guaranteed
            if (!fw.writeF32(dv.px) || !fw.writeF32(dv.py) || !fw.writeF32(dv.pz) ||
                !fw.writeF32(dv.nx) || !fw.writeF32(dv.ny) || !fw.writeF32(dv.nz) ||
                !fw.writeF32(dv.u0) || !fw.writeF32(dv.v0)) {
                if (errorString) *errorString = fw.errorString();
                fw.close();
                return false;
            }
        }

        if (!fw.endChunk(cref)) {
            if (errorString) *errorString = fw.errorString();
            fw.close();
            return false;
        }
    }

    // ─────────────────────────────
    // INDX chunk (u32 indices)
    // ─────────────────────────────
    {
        const auto cref = fw.beginChunk(kChunkINDX);
        if (!cref.valid()) {
            if (errorString) *errorString = fw.errorString();
            fw.close();
            return false;
        }

        for (quint32 idx : mesh.indices) {
            if (!fw.writeU32(idx)) {
                if (errorString) *errorString = fw.errorString();
                fw.close();
                return false;
            }
        }

        if (!fw.endChunk(cref)) {
            if (errorString) *errorString = fw.errorString();
            fw.close();
            return false;
        }
    }

    // ─────────────────────────────
    // SUBM chunk (SubMesh list)
    // ─────────────────────────────
    {
        const auto cref = fw.beginChunk(kChunkSUBM);
        if (!cref.valid()) {
            if (errorString) *errorString = fw.errorString();
            fw.close();
            return false;
        }

        for (const GXSubMeshData &sm : mesh.subMeshes) {
            if (!fw.writeU32(sm.firstIndex) ||
                !fw.writeU32(sm.indexCount) ||
                !fw.writeU32(sm.materialIndex)) {
                if (errorString) *errorString = fw.errorString();
                fw.close();
                return false;
            }
        }

        if (!fw.endChunk(cref)) {
            if (errorString) *errorString = fw.errorString();
            fw.close();
            return false;
        }
    }

    // ─────────────────────────────
    // Optional bounds chunk (BND0)
    // We'll write it only if it looks initialized.
    // (You can make this stricter later.)
    // ─────────────────────────────
    {
        const bool hasBounds =
            !qIsNaN(mesh.boundsMin.x()) && !qIsNaN(mesh.boundsMax.x());

        if (hasBounds) {
            const auto cref = fw.beginChunk(kChunkBND0);
            if (!cref.valid()) {
                if (errorString) *errorString = fw.errorString();
                fw.close();
                return false;
            }

            if (!fw.writeF32(float(mesh.boundsMin.x())) ||
                !fw.writeF32(float(mesh.boundsMin.y())) ||
                !fw.writeF32(float(mesh.boundsMin.z())) ||
                !fw.writeF32(float(mesh.boundsMax.x())) ||
                !fw.writeF32(float(mesh.boundsMax.y())) ||
                !fw.writeF32(float(mesh.boundsMax.z()))) {
                if (errorString) *errorString = fw.errorString();
                fw.close();
                return false;
            }

            if (!fw.endChunk(cref)) {
                if (errorString) *errorString = fw.errorString();
                fw.close();
                return false;
            }
        }
    }

    fw.close();

    if (fw.hasError()) {
        if (errorString) *errorString = fw.errorString();
        return false;
    }

    return true;
}

}
