// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXWORLDQUERYBACKEND_H
#define GXWORLDQUERYBACKEND_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Query/GXRayHit.h>
#include <GenesisX/GX3D/Query/GXRay.h>

namespace gx::gx3d::query {

class GENESISX_GX3D_EXPORT GXWorldQueryBackend
{
public:
    virtual ~GXWorldQueryBackend() = default;
    virtual GXRayHit raycast(const GXRay& ray, int minPriority) const = 0;

};

}

#endif // GXWORLDQUERYBACKEND_H
