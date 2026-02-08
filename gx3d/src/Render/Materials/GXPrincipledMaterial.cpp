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
    QShader s_vs;
    switch (alphaMode()) {
    case GXMaterial::Opaque:
        s_vs = m_shaderUtils.gxLoadShader(":/gx3d/shaders/principled_opaque.vert.qsb");
        break;
    case GXMaterial::Mask:
        s_vs = m_shaderUtils.gxLoadShader(":/gx3d/shaders/principled_mask.vert.qsb");
        break;
    case GXMaterial::Blend:
        s_vs = m_shaderUtils.gxLoadShader(":/gx3d/shaders/principled_blend.vert.qsb");
        break;
    default:
        s_vs = m_shaderUtils.gxLoadShader(":/gx3d/shaders/principled.vert.qsb");
    }

    return s_vs;
}

QShader GXPrincipledMaterial::fragmentShader() const
{
    QShader s_fs;
    switch (alphaMode()) {
    case GXMaterial::Opaque:
        s_fs = m_shaderUtils.gxLoadShader(":/gx3d/shaders/principled_opaque.frag.qsb");
        break;
    case GXMaterial::Mask:
        s_fs = m_shaderUtils.gxLoadShader(":/gx3d/shaders/principled_mask.frag.qsb");
        break;
    case GXMaterial::Blend:
        s_fs = m_shaderUtils.gxLoadShader(":/gx3d/shaders/principled_blend.frag.qsb");
        break;
    default:
        s_fs = m_shaderUtils.gxLoadShader(":/gx3d/shaders/principled.frag.qsb");
        break;
    }

    return s_fs;
}

void GXPrincipledMaterial::applyTo(QRhiGraphicsPipeline *ps) const
{
    ps->setCullMode(QRhiGraphicsPipeline::Back);
    ps->setFrontFace(QRhiGraphicsPipeline::CCW);
    GXMaterial::applyTo(ps);
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

    out.alphaParams[0] = alphaCutoff();
    out.alphaParams[1] = 0.0f;
    out.alphaParams[2] = 0.0f;
    out.alphaParams[3] = 0.0f;

    out.gamma[0] = m_gamma.x();
    out.gamma[1] = m_gamma.y();
    out.gamma[2] = m_gamma.z();
    out.gamma[3] = 1.0f;

    out.normalScale[0] = m_normalScale;
    out.normalScale[1] = 0.0f;
    out.normalScale[2] = 0.0f;
    out.normalScale[3] = 0.0f;
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

void GXPrincipledMaterial::ensureNormalMapResources(QRhi *rhi, QRhiCommandBuffer *cb)
{
    if (!rhi) return;

    if (m_normalTexure) {
        m_normalTexure->ensureRhi(rhi, cb);
        m_normalTex = m_normalTexure->rhiTexture();
        m_normalSampler = m_normalTexure->rhiSampler();
    } else {
        m_normalTexure = nullptr;
        m_normalTex = nullptr;
        m_normalSampler = nullptr;
    }
}

quint32 GXPrincipledMaterial::variantKey() const
{
    quint32 k = 0;
    k |= (quint32(alphaMode()) & 0x3u);
    if (m_baseColorTexture) k |= (1u << 2);

    return k;
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
    // qDebug() << "emCol" << m_emissionColor;
    m_emissionStrength = 1.0;
    emit emissionColorChanged();
    markDirty();
}

void GXPrincipledMaterial::setEmissionStrength(float s)
{
    s = qMax(0.0f, s);
    if (qFuzzyCompare(m_emissionStrength, s)) return;
    m_emissionStrength = s;
    // qDebug() << "emStr" << m_emissionStrength;
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

void GXPrincipledMaterial::setNormalTexture(GXTexture *tex)
{
    if (m_normalTexure == tex) return;
    m_normalTexure = tex;

    emit normalTextureChanged();
    markDirty();
}

void GXPrincipledMaterial::setNormalScale(float s)
{
    if (m_normalScale == s) return;
    m_normalScale = s;

    emit normalScaleChanged();
    markDirty();
}
