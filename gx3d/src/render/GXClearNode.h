// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXCLEARNODE_H
#define GXCLEARNODE_H

#include <QColor>
#include <QVector2D>
#include <QSGRenderNode>

#include <rhi/qrhi.h>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

namespace gx::render {

class GXClearNode final : public QSGRenderNode
{
public:
    GXClearNode();
    ~GXClearNode() override;

    void setRect(const QRectF& r) { m_rect = r; }
    void setColor(const QColor& c) { m_color = c; m_colorDirty = true; }

    QRectF rect() const override { return m_rect; }
    RenderingFlags flags() const override { return BoundedRectRendering; }

    StateFlags changedStates() const override
    {
        return StateFlags(ViewportState | ScissorState | ColorState);
    }

    void prepare() override;
    void render(const RenderState* state) override;
    void releaseResources() override;

private:
    void ensureRhiResources(QRhi* rhi);
    void destroyRhiResources();

    QRectF m_rect;
    QColor m_color = Qt::black;
    bool m_colorDirty = false;

    QRhi* m_rhi = nullptr;

    QRhiBuffer* m_vbuf = nullptr;
    QRhiBuffer* m_ubuf = nullptr;
    QRhiShaderResourceBindings* m_srb = nullptr;
    QRhiGraphicsPipeline* m_ps = nullptr;

    QShader m_vs;
    QShader m_fs;
};

}

#endif // GXCLEARNODE_H
