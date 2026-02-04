// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXSCENEQUERYBACKEND_H
#define GXSCENEQUERYBACKEND_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Query/GXWorldQueryBackend.h>

namespace gx::gx3d::scene { class GXScene; }

namespace gx::gx3d::query {

class GENESISX_GX3D_EXPORT GXSceneQueryBackend final : public GXWorldQueryBackend
{
public:
    explicit GXSceneQueryBackend(scene::GXScene* scene);
    GXRayHit raycast(const GXRay& ray, int minPriority) const override;

private:
    gx::gx3d::scene::GXScene* m_scene = nullptr;
};

}

#endif // GXSCENEQUERYBACKEND_H
