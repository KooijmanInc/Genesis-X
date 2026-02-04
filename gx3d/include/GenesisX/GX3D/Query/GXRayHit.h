// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXRAYHIT_H
#define GXRAYHIT_H

#include <QVector3D>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Scene/Nodes/GXNode.h>

namespace gx::gx3d::query {

struct GXRayHit {
    bool hit = false;
    float t = 0.0f;
    QVector3D positionWS;
    QVector3D normalWS;
    scene::GXNode* node = nullptr;
};

}

#endif // GXRAYHIT_H
