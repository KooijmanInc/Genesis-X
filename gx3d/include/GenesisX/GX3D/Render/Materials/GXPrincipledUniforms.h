// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXPRINCIPLEDUNIFORMS_H
#define GXPRINCIPLEDUNIFORMS_H

#include <QtGlobal>

namespace gx::gx3d::render {

enum : int {
    Principled_VS_Binding = 0,
    Principled_FS_Binding = 1
};

struct alignas(16) PrincipledVSUBO {
    float mvp[16];
    float model[16];
};

struct alignas(16) PrincipledFSUBO {
    float baseColor[4];
    float emission[4];
    float emissionLight[4];
    float alphaParams[4];
    float gamma[4];
    float normalScale[4];
    float metallicFactor[4];
    float roughnessFactor[4];
    float fresnel[4];
    float specular[4];
};

}

#endif // GXPRINCIPLEDUNIFORMS_H
