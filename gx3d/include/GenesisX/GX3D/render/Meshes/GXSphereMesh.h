// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXSPHEREMESH_H
#define GXSPHEREMESH_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>
#include <GenesisX/GX3D/Render/Resources/GXMesh.h>

namespace gx::gx3d::render {

class GENESISX_GX3D_EXPORT GXSphereMesh
{
public:
    static GXMesh* create();
};

}

#endif // GXSPHEREMESH_H
