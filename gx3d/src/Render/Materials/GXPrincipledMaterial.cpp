// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Materials/GXPrincipledMaterial.h>
#include <GenesisX/GX3D/Render/Texture/GXTexture2D.h>

using namespace gx::gx3d::render;

GXPrincipledMaterial::GXPrincipledMaterial(QObject *parent)
    : GXMaterial{parent}
{
    m_solidColorTex = new GXTexture2D(this);
    m_solidColorTex->setDefaultImage(Qt::white);
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

    out.emission[0] = float(m_emissionColor.redF());
    out.emission[1] = float(m_emissionColor.greenF());
    out.emission[2] = float(m_emissionColor.blueF());
    out.emission[3] = float(m_emissionStrength);

    out.emissionLight[0] = float(m_emissionLightIntensity);
    out.emissionLight[1] = float(m_emissionLightRadius);
    out.emissionLight[2] = float(m_emissionLight ? 1.0f : 0.0f);
    out.emissionLight[3] = 0.0f;
}

void GXPrincipledMaterial::ensureBaseColorResources(QRhi *rhi, QRhiCommandBuffer *cb)
{
    if (!rhi) return;

    if (m_baseColorTexture) {
        m_baseColorTexture->ensureRhi(rhi, cb);
        m_baseColorTex = m_baseColorTexture->rhiTexture();
        m_baseColorSampler = m_baseColorTexture->rhiSampler();
    } else {
        m_baseColorTexture = m_solidColorTex;
        m_baseColorTexture->ensureRhi(rhi, cb);
        m_baseColorTex = m_baseColorTexture->rhiTexture();
        m_baseColorSampler = m_baseColorTexture->rhiSampler();
    }
}

void GXPrincipledMaterial::setBaseColor(const QColor &c)
{
    if (m_baseColor == c) return;
    m_baseColor = c;
    m_emissionStrength = 0.0;
    m_solidColorTex->setDefaultImage(c);

    emit baseColorChanged();
    markDirty();
}

void GXPrincipledMaterial::setBaseColorTexture(GXTexture* tex)
{
    if (m_baseColorTexture == tex) return;
    m_baseColorTexture = tex;

    emit baseColorTextureChanged();
    markDirty();
}

void GXPrincipledMaterial::setEmissionColor(const QColor &c)
{
    if (m_emissionColor == c) return;
    m_emissionColor = c;
    qDebug() << "emCol" << m_emissionColor;
    m_emissionStrength = 1.0;
    emit emissionColorChanged();
    markDirty();
}

void GXPrincipledMaterial::setEmissionStrength(float s)
{
    s = qMax(0.0f, s);
    if (qFuzzyCompare(m_emissionStrength, s)) return;
    m_emissionStrength = s;
    qDebug() << "emStr" << m_emissionStrength;
    emit emissionStrengthChanged();
    markDirty();
}

void GXPrincipledMaterial::setEmissionLight(const EmissionLight& l)
{
    if (m_emissionLight == l) return;
    m_emissionLight = l;

    emit emissionLightChanged();
    markDirty();
}

void GXPrincipledMaterial::setEmissionLightIntensity(float l)
{
    if (m_emissionLightIntensity == l) return;
    m_emissionLightIntensity = l;

    emit emissionLightIntensityChanged();
    markDirty();
}

void GXPrincipledMaterial::setEmissionLightRadius(float l)
{
    if (m_emissionLightRadius == l) return;
    m_emissionLightIntensity = l;

    emit emissionLightIntensityChanged();
    markDirty();
}
