// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXDEFAULTLITUNIFORMS_H
#define GXDEFAULTLITUNIFORMS_H

#include <QtGlobal>

namespace gx::gx3d::render {

enum : int {
    DefaultLit_VS_Binding = 0,
    DefaultLit_FS_Binding = 1
};

struct alignas(16) DefaultLitVSUBO {
    float mvp[16];
    float model[16];
};

struct alignas(16) DefaultLitFSUBO {
    float baseColor[4];
    float lightPos[4];
    float lightColor[4];
    float lightParams[4];
};

}

#endif // GXDEFAULTLITUNIFORMS_H
