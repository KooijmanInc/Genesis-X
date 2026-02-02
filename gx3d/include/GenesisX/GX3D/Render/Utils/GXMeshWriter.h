// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXMESHWRITER_H
#define GXMESHWRITER_H

#include <QVector>
#include <QVector2D>
#include <QVector3D>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Render/Utils/GXMeshData.h>

namespace gx::gx3d::render {

// ─────────────────────────────
// Engine-native mesh structures
// ─────────────────────────────

// struct GXVertex
// {
//     QVector3D position;
//     QVector3D normal;
//     QVector2D uv0;
// };

// struct GXSubMeshData
// {
//     quint32 firstIndex = 0;
//     quint32 indexCount = 0;
//     quint32 materialIndex = 0;
// };
// static_assert(sizeof(GXSubMeshData) == 12);
// static_assert(alignof(GXSubMeshData) == 4);

struct GXSubMesh
{
    quint32 firstIndex = 0;
    quint32 indexCount = 0;
    quint32 materialIndex = 0;
};

// struct GXMeshData
// {
//     QVector<GXVertex> vertices;
//     QVector<quint32> indices;
//     QVector<GXSubMeshData> subMeshes;

//     // Optional bounds (recommended). If you don't set these yet,
//     // GXMeshWriter will simply skip writing the bounds chunk.
//     QVector3D boundsMin;
//     QVector3D boundsMax;
// };

// ─────────────────────────────
// Mesh writer interface
// ─────────────────────────────

class GENESISX_GX3D_EXPORT GXMeshWriter
{
public:
    GXMeshWriter() = default;

    // Writes a .gxmesh file.
    // Returns true on success, false on failure.
    bool write(const GXMeshData& mesh, const QString& filePath, QString* errorString) const;

    // Same as write(), but lets you override the version.
    bool writeVersioned(const GXMeshData& mesh, const QString& filePath, quint32 version, QString* errorString = nullptr) const;
};

}

#endif // GXMESHWRITER_H
