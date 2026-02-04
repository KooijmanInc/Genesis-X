// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXWORLDQUERY_H
#define GXWORLDQUERY_H

#include <GenesisX/GX3D/Query/GXWorldQueryBackend.h>
#include <memory>

namespace gx::gx3d::query {

class GENESISX_GX3D_EXPORT GXWorldQuery
{
public:
    void setBackend(std::unique_ptr<GXWorldQueryBackend> backend) { m_backend = std::move(backend); }
    GXRayHit raycast(const GXRay& ray, int minPriority = std::numeric_limits<int>::min()) const;

private:
    std::unique_ptr<GXWorldQueryBackend> m_backend;
};

}

#endif // GXWORLDQUERY_H
