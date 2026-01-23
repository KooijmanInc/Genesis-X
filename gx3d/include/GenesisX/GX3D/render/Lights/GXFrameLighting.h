// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXFRAMELIGHTING_H
#define GXFRAMELIGHTING_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <QVector3D>

namespace gx::gx3d::render {

struct GXPointLightData
{
    QVector3D positionWS { 0, 2, 2 };
    float intensity = 1.0f;
    QVector3D color { 1, 1, 1 };
    float range = 10.0f;
};

}

#endif // GXFRAMELIGHTING_H
