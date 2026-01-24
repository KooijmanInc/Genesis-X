// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Materials/GXPrincipledMaterial.h>

using namespace gx::gx3d::render;

GXPrincipledMaterial::GXPrincipledMaterial(QObject *parent)
    : GXMaterial{parent}
{
}

QShader GXPrincipledMaterial::vertexShader() const
{
    static QShader s_vs = m_shaderUtils.gxLoadShader(":/gx3d/shaders/principled.vert.qsb");
    return s_vs;
}

QShader GXPrincipledMaterial::fragmentShader() const
{
    static QShader s_fs = m_shaderUtils.gxLoadShader(":/gx3d/shaders/principled.frag.qsb");
    return s_fs;
}

void GXPrincipledMaterial::applyTo(QRhiGraphicsPipeline *ps) const
{
    ps->setCullMode(QRhiGraphicsPipeline::Back);
    ps->setFrontFace(QRhiGraphicsPipeline::CCW);
    ps->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
    ps->setDepthTest(true);
    ps->setDepthWrite(true);
}

void GXPrincipledMaterial::fillVS(void* dst, const QMatrix4x4 &mvp, const QMatrix4x4 &model) const
{
    auto& out = *reinterpret_cast<PrincipledVSUBO*>(dst);
    memcpy(out.mvp, mvp.constData(), 16 * sizeof(float));
    memcpy(out.model, model.constData(), 16 * sizeof(float));
}

void GXPrincipledMaterial::fillFS(void* dst) const
{
    auto& out = *reinterpret_cast<PrincipledFSUBO*>(dst);
    out.baseColor[0] = float(m_baseColor.redF());
    out.baseColor[1] = float(m_baseColor.greenF());
    out.baseColor[2] = float(m_baseColor.blueF());
    out.baseColor[3] = float(m_baseColor.alphaF());

    out.emissive[0] = float(m_emissiveColor.redF());
    out.emissive[1] = float(m_emissiveColor.greenF());
    out.emissive[2] = float(m_emissiveColor.blueF());
    out.emissive[3] = float(m_emissiveStrength);

    out.emissiveLight[0] = float(m_emissiveLightIntensity);
    out.emissiveLight[1] = float(m_emissiveLightRadius);
    out.emissiveLight[2] = float(m_emissiveLight ? 1.0f : 0.0f);
    out.emissiveLight[3] = 0.0f;
}

void GXPrincipledMaterial::setBaseColor(const QColor &c)
{
    if (m_baseColor == c) return;
    m_baseColor = c;

    emit baseColorChanged();
}

void GXPrincipledMaterial::setEmissiveColor(const QColor &c)
{
    if (m_emissiveColor == c) return;
    m_emissiveColor = c;

    emit emissiveColorChanged();
}

void GXPrincipledMaterial::setEmissiveStrength(float s)
{
    if (m_emissiveStrength == s) return;
    m_emissiveStrength = s;

    emit emissiveStrengthChanged();
}

void GXPrincipledMaterial::setEmissiveLight(bool l)
{
    if (m_emissiveLight == l) return;
    m_emissiveLight = l;

    emit emissiveLightChanged();
}

void GXPrincipledMaterial::setEmissiveLightIntensity(float l)
{
    if (m_emissiveLightIntensity == l) return;
    m_emissiveLightIntensity = l;

    emit emissiveLightIntensityChanged();
}

void GXPrincipledMaterial::setEmissiveLightRadius(float l)
{
    if (m_emissiveLightRadius == l) return;
    m_emissiveLightIntensity = l;

    emit emissiveLightIntensityChanged();
}
