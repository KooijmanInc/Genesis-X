// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Query/GXWorldQuery.h>

namespace  gx::gx3d::query {

GXRayHit GXWorldQuery::raycast(const GXRay &ray, int minPriority) const
{
    return m_backend ? m_backend->raycast(ray, minPriority) : GXRayHit();
}



}
