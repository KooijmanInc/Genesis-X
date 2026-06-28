// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Nodes/GXRenderableNode.h>

using namespace gx::gx3d::render;

GXRenderableNode::GXRenderableNode(QObject *parent)
    : GXNode{parent}
{
}

GXRenderableNode::~GXRenderableNode()
{
    // qDebug() << "~GXRenderableNode" << this << "thread" << QThread::currentThread();
    releaseResourcesBase();
}

void GXRenderableNode::syncFromScene()
{
}

void GXRenderableNode::ensureResources(QRhi *rhi, QRhiRenderTarget *rt, QRhiCommandBuffer* cb)
{
    Q_UNUSED(cb);
    if (m_rhi && m_rhi != rhi) releaseResources();

    m_rhi = rhi;

    if (rt) {
        auto *rp = rt->renderPassDescriptor();
        const int sc = rt->sampleCount();
        const QSize ps = rt->pixelSize();
        if (rp != m_lastRpDesc || sc != m_lastRtSampleCount) {
            m_lastRpDesc = rp;
            m_lastRtSampleCount = sc;
            m_lastRtPixelSize = ps;
            invalidPipeline();
        }
    }
}

void GXRenderableNode::recordRender(QRhiCommandBuffer */*cb*/, QRhiRenderTarget */*rt*/, const QRect &/*scissor*/)
{

}

void GXRenderableNode::releaseResources()
{
    releaseResourcesBase();
}

void GXRenderableNode::setFrameLightingUbo(QRhiBuffer *ubo)
{
    if (m_frameLightingUbo == ubo) return;
    m_frameLightingUbo = ubo;

    invalidPipeline();
}

void GXRenderableNode::setFrameEnvironmentUbo(QRhiBuffer *ubo)
{
    if (m_environmentUbo == ubo) return;
    m_environmentUbo = ubo;

    invalidPipeline();
}

void GXRenderableNode::setBrdfLutTex(QRhiTexture *tex)
{
    if (m_brdfLutTex == tex) return;
    m_brdfLutTex = tex;

    invalidPipeline();
}

void GXRenderableNode::setBrdfLutSampler(QRhiSampler *sampler)
{
    if (m_brdfLutSampler == sampler) return;
    m_brdfLutSampler = sampler;

    invalidPipeline();
}

void GXRenderableNode::setEnvCubeTex(QRhiTexture *tex)
{
    if (m_envCubeTex == tex) return;
    m_envCubeTex = tex;

    invalidPipeline();
}

void GXRenderableNode::setEnvCubeSampler(QRhiSampler *sampler)
{
    if (m_envCubeSampler == sampler) return;
    m_envCubeSampler = sampler;

    invalidPipeline();
}

void GXRenderableNode::setPrefilterSpecCubeTex(QRhiTexture *tex)
{
    if (m_prefilterSpecCubeTex == tex) return;
    m_prefilterSpecCubeTex = tex;

    invalidPipeline();
}

void GXRenderableNode::setPrefilterSpecCubeSampler(QRhiSampler *sampler)
{
    if (m_prefilterSpecCubeSampler == sampler) return;
    m_prefilterSpecCubeSampler = sampler;

    invalidPipeline();
}

void GXRenderableNode::setIrradianceCubeTex(QRhiTexture *tex)
{
    if (m_irradianceCubeTex == tex) return;
    m_irradianceCubeTex = tex;

    invalidPipeline();
}

void GXRenderableNode::setIrradianceCubeSampler(QRhiSampler *sampler)
{
    if (m_irradianceCubeSampler == sampler) return;
    m_irradianceCubeSampler = sampler;

    invalidPipeline();
}

void GXRenderableNode::setMaterial(GXMaterial *m)
{
    if (m_material == m) return;

    if (m_material) disconnect(m_material, nullptr, this, nullptr);

    m_material = m;

    if (m_material) {
        if (m && m->parent() == nullptr) m->setParent(this);
        connect(m_material, &GXMaterial::renderStateChanged, this, &GXRenderableNode::invalidPipeline);
    }

    emit materialChanged(m_material);
    invalidPipeline();
}

void GXRenderableNode::setViewProj(const QMatrix4x4 vp)
{
    m_viewProj = vp;
}

void GXRenderableNode::setPointLight(const GXPointLightData &l)
{
    m_pointLight = l;
}

void GXRenderableNode::releaseResourcesBase()
{
    m_rhi = nullptr;
    m_lastRpDesc = nullptr;
    m_lastRtSampleCount = 0;
    m_lastRtPixelSize = QSize();
    m_pipelineDirty = true;
}

void GXRenderableNode::invalidPipeline()
{
    m_pipelineDirty = true;

    emit renderDirty();
}

void GXRenderableNode::markForRelease()
{
    m_pendingRelease = true;
    m_pipelineDirty = true;
}

void GXRenderableNode::setScene(scene::GXScene *s)
{
    m_scene = s;
}
