// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Nodes/GXRenderableNode.h>
// #include <GenesisX/GX3D/Render/Materials/GXMaterial.h>

using namespace gx::gx3d::render;

GXRenderableNode::GXRenderableNode(QObject *parent)
    : GXNode{parent}
{
    // m_material = new GXMaterial(this);

    // connect(m_material, &GXMaterial::renderStateChanged, this, &GXRenderableNode::invalidPipeline);
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
        if (rp != m_lastRpDesc || sc != m_lastRtSampleCount) {
            // qWarning() << "RT changed for" << this
            //            << "rp" << rp << "last" << m_lastRpDesc
            //            << "sc" << sc << "lastSc" << m_lastRtSampleCount;
            m_lastRpDesc = rp;
            m_lastRtSampleCount = sc;
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

void GXRenderableNode::setMaterial(GXMaterial *m)
{
    qWarning() << "releaseResources called on" << this;
    if (m_material == m) return;

    // qWarning().noquote() << "setMaterial:"
    //                      << "this=" << this
    //                      << "old=" << m_material
    //                      << "new=" << m
    //                      << "new.meta=" << (m ? m->metaObject()->className() : "null")
    //                      << "new.parent=" << (m ? m->parent() : nullptr);

    if (m_material) disconnect(m_material, nullptr, this, nullptr);

    m_material = m;

    if (m_material) {
        if (m && m->parent() == nullptr) m->setParent(this);
        connect(m_material, &GXMaterial::renderStateChanged, this, &GXRenderableNode::invalidPipeline);
        // qDebug() << "is this keep calling it? set material";
    }

    emit materialChanged(m_material);
    invalidPipeline();
    // qDebug() << "is this keep calling it? set material second";
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
}

void GXRenderableNode::invalidPipeline()
{
    if (m_pipelineDirty) return;
    m_pipelineDirty = true;

    emit renderDirty();
}
