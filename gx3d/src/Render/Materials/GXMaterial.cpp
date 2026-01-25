// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Materials/GXMaterial.h>

using namespace gx::gx3d::render;

GXMaterial::GXMaterial(QObject *parent)
    : QObject{parent}
{
    // m_vs = m_shaderUtils.gxLoadShader(":/gx3d/shaders/default_lit.vert.qsb");
    // m_fs = m_shaderUtils.gxLoadShader(":/gx3d/shaders/default_lit.frag.qsb");
}

void GXMaterial::setRenderState(const GXRenderState &s)
{
    if (m_state == s) return;
    m_state = s;

    emit renderStateChanged();

    markDirty();
}

void GXMaterial::setDoubleSided(bool on)
{
    const auto newMode = on ? QRhiGraphicsPipeline::None : QRhiGraphicsPipeline::Back;
    if (m_state.cullMode == newMode) return;
    m_state.cullMode = newMode;

    emit renderStateChanged();

    markDirty();
}

void GXMaterial::setDepthTest(bool on)
{
    if (m_state.depthTest == on) return;
    m_state.depthTest = on;

    emit renderStateChanged();

    markDirty();
}

void GXMaterial::setDepthWrite(bool on)
{
    if (m_state.depthWrite == on) return;
    m_state.depthWrite = on;

    emit renderStateChanged();

    markDirty();
}

GXMaterial::CullMode GXMaterial::cullMode() const
{
    switch (m_state.cullMode) {
    case QRhiGraphicsPipeline::None: return CullNone;
    case QRhiGraphicsPipeline::Front: return CullFront;
    case QRhiGraphicsPipeline::Back: return CullBack;
    default: return CullBack;
    }
}

void GXMaterial::setCullMode(CullMode m)
{
    QRhiGraphicsPipeline::CullMode cm = QRhiGraphicsPipeline::Back;
    switch (m) {
    case CullNone: cm = QRhiGraphicsPipeline::None; break;
    case CullFront: cm = QRhiGraphicsPipeline::Front; break;
    case CullBack: cm = QRhiGraphicsPipeline::Back; break;
    }
    if (m_state.cullMode == cm) return;
    m_state.cullMode = cm;

    emit renderStateChanged();

    markDirty();
}

void GXMaterial::setFrontFace(FrontFace f)
{
    const auto ff = (f == FrontCW) ? QRhiGraphicsPipeline::CW : QRhiGraphicsPipeline::CCW;
    if (m_state.frontFace == ff) return;
    m_state.frontFace = ff;

    emit renderStateChanged();

    markDirty();
}

bool GXMaterial::consumeDirty()
{
    const bool was = m_dirty;
    m_dirty = false;
    return was;
}

void GXMaterial::ensureRhi(QRhi *rhi, QRhiCommandBuffer *cb)
{
    if (!rhi) return;

    if (m_rhi != rhi) {
        destroyRhiResources();
        m_rhi = rhi;
        markDirty();
    }

    ensureBaseColorResources(rhi, cb);
}

void GXMaterial::markDirty()
{
    if (m_dirty) return;
    m_dirty = true;
    emit materialChanged();
}

void GXMaterial::destroyRhiResources()
{
    if (m_baseColorTex) {
        m_baseColorTex->destroy();
        delete m_baseColorTex;
        m_baseColorTex = nullptr;
    }
    if (m_baseColorSampler) {
        m_baseColorSampler->destroy();
        delete m_baseColorSampler;
        m_baseColorSampler = nullptr;
    }
}

GXMaterial::FrontFace GXMaterial::frontFace() const
{
    return (m_state.frontFace == QRhiGraphicsPipeline::CW) ? FrontCW : FrontCCW;
}

void GXMaterial::applyTo(QRhiGraphicsPipeline *ps) const
{
    if (!ps) return;

    ps->setCullMode(m_state.cullMode);
    ps->setFrontFace(m_state.frontFace);

    ps->setDepthTest(m_state.depthTest);
    ps->setDepthWrite(m_state.depthWrite);
    ps->setDepthOp(m_state.depthOp);

    QRhiGraphicsPipeline::TargetBlend blend;
    blend.enable = m_state.blending;
    ps->setTargetBlends({ blend });
}
