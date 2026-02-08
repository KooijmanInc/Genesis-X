// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXMESHDATA_H
#define GXMESHDATA_H

#include <QVector>
#include <QVector2D>
#include <QVector3D>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

namespace gx::gx3d::render {

struct GXVertex
{
    QVector3D position;
    QVector3D normal;
    QVector2D uv0;
    QVector4D tangent;
};

struct GXSubMeshData
{
    quint32 firstIndex = 0;
    quint32 indexCount = 0;
    quint32 materialIndex = 0;
};

struct GXMeshData
{
    QVector<GXVertex> vertices;
    QVector<quint32> indices;
    QVector<GXSubMeshData> subMeshes;

    QVector3D boundsMin = QVector3D(qQNaN(), qQNaN(), qQNaN());
    QVector3D boundsMax = QVector3D(qQNaN(), qQNaN(), qQNaN());
};

}

#endif // GXMESHDATA_H
