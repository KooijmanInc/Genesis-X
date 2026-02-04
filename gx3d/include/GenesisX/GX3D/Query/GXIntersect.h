// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXINTERSECT_H
#define GXINTERSECT_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Query/GXRay.h>
#include <GenesisX/GX3D/Query/GXAabb.h>

namespace gx::gx3d::query {

GENESISX_GX3D_EXPORT bool intersectRayAabb(const GXRay& ray, const GXAabb& box, float & outT, QVector3D& outNormal);

}

#endif // GXINTERSECT_H
