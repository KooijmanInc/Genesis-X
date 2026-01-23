// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXTESTTRIANGLENODE_H
#define GXTESTTRIANGLENODE_H

#include <QColor>
#include <QMatrix4x4>
#include <QSGRenderNode>
#include <rhi/qrhi.h>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

namespace gx::test {

class GENESISX_GX3D_EXPORT GXTestTriangleNode : public QSGRenderNode
{
public:
    GXTestTriangleNode();
    ~GXTestTriangleNode() override;

    void setRect(const QRectF &r) { m_rect = r; }
    void setColor(const QColor &c) { m_color = c; }
    void setMvp(const QMatrix4x4 &mvp) { m_mvp = mvp; }

    QRectF rect() const override { return m_rect; }
    RenderingFlags flags() const override { return BoundedRectRendering; }
    StateFlags changedStates() const override { return StateFlags(ViewportState | ScissorState); }

    void render(const RenderState *state) override;
    void releaseResources() override { destroy(); }

private:
    void ensure(QRhi *rhi, QRhiRenderTarget *rt);
    void destroy();

    QRectF m_rect;
    QColor m_color = Qt::white;
    QMatrix4x4 m_mvp;

    QRhi *m_rhi = nullptr;

    QRhiBuffer *m_vbuf = nullptr;
    QRhiBuffer *m_ubuf = nullptr;
    QRhiShaderResourceBindings *m_srb = nullptr;
    QRhiGraphicsPipeline *m_ps = nullptr;

    QShader m_vs, m_fs;
};

}

#endif // GXTESTTRIANGLENODE_H
