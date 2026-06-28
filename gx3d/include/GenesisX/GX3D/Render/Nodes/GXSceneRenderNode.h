// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXSCENERENDERNODE_H
#define GXSCENERENDERNODE_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Scene/Lights/GXSpotLight.h>
#include <GenesisX/GX3D/Scene/Lights/GXPointLight.h>
#include <GenesisX/GX3D/Render/Lights/GXFrameLighting.h>
#include <GenesisX/GX3D/Render/Utils/GXShaderUtils.h>
#include <GenesisX/GX3D/Render/Environment/GXEnvironment.h>

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

struct FrameStats {
    int pipelinesCreated = 0;
};

class GENESISX_GX3D_EXPORT GXSceneRenderNode : public QSGRenderNode
{
public:
    GXSceneRenderNode();
    ~GXSceneRenderNode() override;

    void setRect(const QRectF& r) { m_rect = r; markDirty(DirtyGeometry); }
    void setScene(scene::GXScene* scene);
    void setViewProj(const QMatrix4x4& vp) { m_viewProj = vp; markDirty(DirtyGeometry); }
    void setCameraWorldPos(const QVector3D& cwp) { m_cameraWorldPos = cwp; }
    void setEnvironment(render::GXEnvironment* env) {
        if (m_environment == env) return;
        m_environment = env;
        m_environmentDirty = true;
    }

    void setQuickWindow(QQuickWindow* w);

    void render(const RenderState* state) override;
    StateFlags changedStates() const override { return ScissorState | ViewportState; }
    RenderingFlags flags() const override { return BoundedRectRendering; }
    QRectF rect() const override { return m_rect; }
    void setWindowRectPx(const QRect& r) { m_windowRectPx = r; }

    void syncRenderables();
    FrameStats m_stats;

    // QRhiTexture* pickTexture() const { return m_pickTex; }
    // QRhiTextureRenderTarget* pickRenderTarget() const { return m_pickRt; }

public slots:
    void requestRender();

protected:
    void prepare() override;


private:
    QRectF m_rect;
    QRect m_windowRectPx;
    QMatrix4x4 m_viewProj;
    QVector3D m_cameraWorldPos;
    scene::GXScene* m_scene = nullptr;
    render::GXEnvironment* m_environment = nullptr;
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

    QRhiBuffer* m_environmentUbo = nullptr;
    bool m_environmentDirty = true;

    QSize m_lastSize;
    int m_lastSampleCount = 1;

    void ensureDepthTarget(QRhi *rhi, QRhiRenderTarget *windowRt);
    void destroyDepthTarget();

    void ensureLightGizmo(QRhi* rhi, QRhiCommandBuffer* cb, QRhiRenderTarget* rt);
    void destroyLightGizmo();

    QRhiTexture* m_brdfLutTex = nullptr;
    QRhiSampler* m_brdfLutSampler = nullptr;

    QRhiTexture* m_envCubeTex = nullptr;
    QRhiSampler* m_envCubeSampler = nullptr;

    QRhiTexture* m_prefilterSpecCubeTex = nullptr;
    QRhiSampler* m_prefilterSpecCubeSampler = nullptr;
    QRhiBuffer *m_prefilterParamsUbo = nullptr;
    QRhiGraphicsPipeline *m_prefilterPipeline = nullptr;
    QRhiShaderResourceBindings *m_prefilterSrb = nullptr;
    QRhiRenderPassDescriptor *m_prefilterRp = nullptr;
    // bool m_prefilterParamsCreated = false;
    int m_prefilterSamplesDesktop = 256;
    int m_prefilterSamplesMobile  = 64;

    int m_brdfInvSize = 256;

    QRhiTexture* m_irradianceCubeTex = nullptr;
    QRhiSampler* m_irradianceCubeSampler = nullptr;

    void ensureBrdfLut(QRhi* rhi, QRhiCommandBuffer* cb);
    bool m_brdfLutUploaded = false;

    void ensureEnvCube(QRhi* rhi, QRhiCommandBuffer* cb);
    bool m_envCubeUploaded = false;

    void ensurePrefilterSpecCube(QRhi* rhi, QRhiCommandBuffer* cb);
    bool m_prefilterSpecCubeBuilt = false;

    void ensurePrefilterPipeline(QRhi* rhi, QRhiRenderPassDescriptor *rp);
    void ensurePrefilterSrb(QRhi* rhi);
    void ensurePrefilterParamsUbo(QRhi* rhi);
    void renderIntoCubeFaceMip(QRhi* rhi, QRhiCommandBuffer* cb, int face, int mip, int mipSize, float roughness);
    void updatePrefilterParams(QRhi* rhi, QRhiCommandBuffer* cb, int face, float roughness);

    void ensureIrradianceCube(QRhi* rhi, QRhiCommandBuffer* cb);
    bool m_irradianceCubeUploaded = false;

    void destroyFrameLightUbo();

    void destroyEnvironmentUbo();

    QMetaObject::Connection m_beforeRenderingConn;
    QMetaObject::Connection m_sceneGraphInvalidatedConn;
    std::atomic_bool m_envBuildRequested { true };   // or false, your choice
    // std::atomic_bool m_envBuilt { false };
    bool m_envBuilt = false;

    // int m_renderTimes = 1;

    // void buildEnvironment();

    // void ensurePickTarget(QRhi* rhi, QRhiRenderTarget* rt);
    // void destroyPickTarget();
    // void ensurePickPassResources(QRhi* rhi);
    // void destroyPickPassResources();

    std::atomic_bool m_depthDirty { true };

    float m_maxMip = 7.0;

    mutable QVector<QPointer<GXRenderableNode>> m_renderables;



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
