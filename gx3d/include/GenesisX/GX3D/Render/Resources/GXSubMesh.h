// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXSUBMESH_H
#define GXSUBMESH_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Render/Materials/GXMaterial.h>

namespace gx::gx3d::render {

class GENESISX_GX3D_EXPORT GXSubMesh
{
public:
    quint32 indexOffset = 0;
    quint32 indexCount = 0;
    int materialSlot = -1;
    quint32 baseVertex = 0;
};

}

#endif // GXSUBMESH_H
