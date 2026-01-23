// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXSHADERUTILS_H
#define GXSHADERUTILS_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <rhi/qshader.h>

namespace gx::gx3d::render {

class GENESISX_GX3D_EXPORT GXShaderUtils
{
public:
    GXShaderUtils() = default;

    QShader gxLoadShader(const QString& path) const;
};

}

#endif // GXSHADERUTILS_H
