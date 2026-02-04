// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXSCENERENDERNODE_H
#define GXSCENERENDERNODE_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Scene/Lights/GXSpotLight.h>
#include <GenesisX/GX3D/Scene/Lights/GXPointLight.h>
#include <GenesisX/GX3D/Render/Lights/GXFrameLighting.h>
#include <GenesisX/GX3D/Render/Utils/GXShaderUtils.h>

#include <QSGRenderNode>
#include <QQuickWindow>
#include <QMatrix4x4>

#include <rhi/qrhi.h>
#include <atomic>
#include <GenesisX/GX3D/Render/Utils/GXShaderUtils.h>

namespace gx::gx3d::scene {
class GXScene;
class GXNode;
}
namespace gx::gx3d::render {
class GXRenderableNode;

class GENESISX_GX3D_EXPORT GXSceneRenderNode : public QSGRenderNode
{
public:
    GXSceneRenderNode();
    ~GXSceneRenderNode() override;

    void setRect(const QRectF& r) { m_rect = r; markDirty(DirtyGeometry); }
    void setScene(scene::GXScene* scene);
    void setViewProj(const QMatrix4x4& vp) { m_viewProj = vp; markDirty(DirtyGeometry); }

    void setQuickWindow(QQuickWindow* w);

    void render(const RenderState* state) override;
    StateFlags changedStates() const override { return {}; }
    RenderingFlags flags() const override { return BoundedRectRendering; }
    QRectF rect() const override { return m_rect; }

    // QRhiTexture* pickTexture() const { return m_pickTex; }
    // QRhiTextureRenderTarget* pickRenderTarget() const { return m_pickRt; }

public slots:
    void requestRender();

private:
    QRectF m_rect;
    QMatrix4x4 m_viewProj;
    scene::GXScene* m_scene = nullptr;
    GXShaderUtils m_shaderUtils;

    QQuickWindow* m_window = nullptr;

    void forEachRenderable(const std::function<void(GXRenderableNode*)>& fn) const;

    // QRhiTexture *m_depthTex = nullptr;
    QRhiRenderBuffer *m_depthBuffer = nullptr;
    QRhiTextureRenderTarget *m_rtWithDepth = nullptr;
    QRhiRenderPassDescriptor *m_rpDesc = nullptr;
    QRhiSwapChain* m_lastSwapChain = nullptr;

    // light debug
    QRhi* m_gizmoRhi = nullptr;
    QRhiBuffer* m_gizmoVbuf = nullptr;
    QRhiBuffer* m_gizmoUbuf = nullptr;
    QRhiShaderResourceBindings* m_gizmoSrb = nullptr;
    QRhiGraphicsPipeline* m_gizmoPs = nullptr;
    bool m_gizmoDirty = true;

    // lighting
    QRhi* m_lastRhi = nullptr;
    QRhiBuffer* m_frameLightUbo = nullptr;
    bool m_frameLightDirty = true;

    QSize m_lastSize;
    int m_lastSampleCount = 1;

    void ensureDepthTarget(QRhi *rhi, QRhiRenderTarget *windowRt);
    void destroyDepthTarget();

    void ensureLightGizmo(QRhi* rhi, QRhiCommandBuffer* cb, QRhiRenderTarget* rt);
    void destroyLightGizmo();

    void destroyFrameLightUbo();

    // void ensurePickTarget(QRhi* rhi, QRhiRenderTarget* rt);
    // void destroyPickTarget();
    // void ensurePickPassResources(QRhi* rhi);
    // void destroyPickPassResources();

    std::atomic_bool m_depthDirty { true };

    // QRhiTexture* m_pickTex = nullptr;
    // QRhiTextureRenderTarget* m_pickRt = nullptr;
    // QRhiRenderPassDescriptor* m_pickRp = nullptr;
    // QMetaObject::Connection m_beforeRenderingConn;

    // QRhiGraphicsPipeline* m_pickPs = nullptr;
    // QRhiShaderResourceBindings* m_pickSrb = nullptr;
    // QRhiBuffer* m_pickUbuf = nullptr;

    // QRhi* m_pickRhi = nullptr;

    // QSize m_lastPickSize;
    // int m_lastPickSampleCount = 1;

    // GXShaderUtils renderUtils;
};

}

#endif // GXSCENERENDERNODE_H
