// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXRENDERABLENODE_H
#define GXRENDERABLENODE_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Scene/GXScene.h>
#include <GenesisX/GX3D/Scene/Nodes/GXNode.h>
#include <GenesisX/GX3D/Render/Materials/GXMaterial.h>
#include <GenesisX/GX3D/Render/Lights/GXFrameLighting.h>

#include <rhi/qrhi.h>

class QRhi;
class QRhiRenderTarget;
class QRhiCommandBuffer;

namespace gx::gx3d::render {

class GXMaterial;

// std::atomic_int m_renderHold {0};
// std::atomic_bool m_deleteRequested {false};

class GENESISX_GX3D_EXPORT GXRenderableNode : public gx::gx3d::scene::GXNode
{
    Q_OBJECT

    Q_PROPERTY(GXMaterial* material READ material WRITE setMaterial NOTIFY materialChanged)

public:
    explicit GXRenderableNode(QObject* parent = nullptr);
    ~GXRenderableNode() override;

    virtual void syncFromScene();

    virtual void ensureResources(QRhi* rhi, QRhiRenderTarget* rt, QRhiCommandBuffer* cb);

    virtual void recordRender(QRhiCommandBuffer* cb, QRhiRenderTarget* rt, const QRect &scissor) = 0;

    virtual void releaseResources();

    virtual void setFrameLightingUbo(QRhiBuffer* ubo);
    virtual void setFrameEnvironmentUbo(QRhiBuffer* ubo);

    virtual void setBrdfLutTex(QRhiTexture* tex);
    virtual void setBrdfLutSampler(QRhiSampler* sampler);

    virtual void setEnvCubeTex(QRhiTexture* tex);
    virtual void setEnvCubeSampler(QRhiSampler* sampler);

    virtual void setPrefilterSpecCubeTex(QRhiTexture* tex);
    virtual void setPrefilterSpecCubeSampler(QRhiSampler* sampler);

    virtual void setIrradianceCubeTex(QRhiTexture* tex);
    virtual void setIrradianceCubeSampler(QRhiSampler* sampler);

    GXMaterial* material() const { return m_material; }
    void setMaterial(GXMaterial* m);

    void setViewProj(const QMatrix4x4 vp);

    void setPointLight(const GXPointLightData& l);

    void markForRelease();

    void setScene(scene::GXScene* s);

    // void acquireRenderHold() { m_renderHold.fetch_add(1, std::memory_order_relaxed); }
    // void releaseRenderHold() { m_renderHold.fetch_sub(1, std::memory_order_relaxed); }
    // bool canDeleteNow() const { return m_renderHold.load(std::memory_order_relaxed) == 0; }
    // void requestDelete() { m_deleteRequested.store(true, std::memory_order_relaxed); }
    // bool deleteRequested() const { return m_deleteRequested.load(std::memory_order_relaxed); }

signals:
    void materialChanged(gx::gx3d::render::GXMaterial* material);
    void renderDirty();

protected:
    void releaseResourcesBase();
    void invalidPipeline();
    QRhi* m_rhi = nullptr;
    QRhiRenderPassDescriptor* m_lastRpDesc = nullptr;
    int m_lastRtSampleCount = 0;

    GXMaterial* m_material = nullptr;
    bool m_pipelineDirty = true;
    QMatrix4x4 m_viewProj;

    QRhiBuffer* m_frameLightingUbo = nullptr;
    QRhiBuffer* m_environmentUbo = nullptr;

    QRhiTexture* m_brdfLutTex = nullptr;
    QRhiSampler* m_brdfLutSampler = nullptr;

    QRhiTexture* m_envCubeTex = nullptr;
    QRhiSampler* m_envCubeSampler = nullptr;

    QRhiTexture* m_prefilterSpecCubeTex = nullptr;
    QRhiSampler* m_prefilterSpecCubeSampler = nullptr;

    QRhiTexture* m_irradianceCubeTex = nullptr;
    QRhiSampler* m_irradianceCubeSampler = nullptr;

    QSize m_lastRtPixelSize;

    bool m_pendingRelease = false;

    GXPointLightData m_pointLight;

    scene::GXScene* m_scene = nullptr;
};

}

#endif // GXRENDERABLENODE_H
