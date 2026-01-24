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
    releaseResourcesBase();
}

void GXRenderableNode::syncFromScene()
{
}

void GXRenderableNode::ensureResources(QRhi *rhi, QRhiRenderTarget *rt)
{
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

void GXRenderableNode::recordRender(QRhiCommandBuffer */*cb*/, QRhiRenderTarget */*rt*/)
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
