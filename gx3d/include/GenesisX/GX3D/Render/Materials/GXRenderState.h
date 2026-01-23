// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXRENDERSTATE_H
#define GXRENDERSTATE_H

#include <rhi/qrhi.h>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

namespace gx::gx3d::render {

struct GXRenderState
{
    QRhiGraphicsPipeline::CullMode cullMode = QRhiGraphicsPipeline::Back;
    QRhiGraphicsPipeline::FrontFace frontFace = QRhiGraphicsPipeline::CCW;

    bool depthTest = true;
    bool depthWrite = true;
    QRhiGraphicsPipeline::CompareOp depthOp = QRhiGraphicsPipeline::LessOrEqual;

    bool blending = false;

    bool operator==(const GXRenderState& o) const
    {
        return cullMode == o.cullMode
            && frontFace == o.frontFace
            && depthTest == o.depthTest
            && depthWrite == o.depthWrite
            && depthOp == o.depthOp
            && blending == o.blending;
    }
    bool operator!=(const GXRenderState& o) const { return !(*this == o); }
};

}

#endif // GXRENDERSTATE_H
