// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXRENDERABLENODE_H
#define GXRENDERABLENODE_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Scene/Nodes/GXNode.h>
#include <GenesisX/GX3D/Render/Materials/GXMaterial.h>
#include <GenesisX/GX3D/Render/Lights/GXFrameLighting.h>

#include <rhi/qrhi.h>

class QRhi;
class QRhiRenderTarget;
class QRhiCommandBuffer;

namespace gx::gx3d::render {

class GXMaterial;

class GENESISX_GX3D_EXPORT GXRenderableNode : public gx::gx3d::scene::GXNode
{
    Q_OBJECT

    Q_PROPERTY(GXMaterial* material READ material WRITE setMaterial NOTIFY materialChanged)

public:
    explicit GXRenderableNode(QObject* parent = nullptr);
    ~GXRenderableNode() override;

    virtual void syncFromScene();

    virtual void ensureResources(QRhi* rhi, QRhiRenderTarget* rt);

    virtual void recordRender(QRhiCommandBuffer* cb, QRhiRenderTarget* rt);

    virtual void releaseResources();

    virtual void setFrameLightingUbo(QRhiBuffer* ubo);

    GXMaterial* material() const { return m_material; }
    void setMaterial(GXMaterial* m);

    void setViewProj(const QMatrix4x4 vp);

    void setPointLight(const GXPointLightData& l);

    void markForRelease();

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

    QSize m_lastRtPixelSize;

    bool m_pendingRelease = false;

    GXPointLightData m_pointLight;
};

}

#endif // GXRENDERABLENODE_H
