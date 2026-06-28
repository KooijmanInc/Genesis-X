// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXPRINCIPLEDMATERIAL_H
#define GXPRINCIPLEDMATERIAL_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Render/Materials/GXMaterial.h>
#include <GenesisX/GX3D/Render/Materials/GXPrincipledUniforms.h>

#include <GenesisX/GX3D/Render/Texture/GXTexture.h>
#include <GenesisX/GX3D/Render/Texture/GXTexture2D.h>

namespace gx::gx3d::render {

class GENESISX_GX3D_EXPORT GXPrincipledMaterial : public GXMaterial
{
    Q_OBJECT

    Q_PROPERTY(QColor baseColor READ baseColor WRITE setBaseColor NOTIFY baseColorChanged)
    Q_PROPERTY(GXTexture* baseColorTexture READ baseColorTexture WRITE setBaseColorTexture NOTIFY baseColorTextureChanged)
    Q_PROPERTY(QColor emissionColor READ emissionColor WRITE setEmissionColor NOTIFY emissionColorChanged)
    Q_PROPERTY(float emissionStrength READ emissionStrength WRITE setEmissionStrength NOTIFY emissionStrengthChanged)
    Q_PROPERTY(EmissionLight emissionLight READ emissionLight WRITE setEmissionLight NOTIFY emissionLightChanged)
    Q_PROPERTY(float emissionLightIntensity READ emissionLightIntensity WRITE setEmissionLightIntensity NOTIFY emissionLightIntensityChanged)
    Q_PROPERTY(float emissionLightRadius READ emissionLightRadius WRITE setEmissionLightRadius NOTIFY emissionLightRadiusChanged)
    Q_PROPERTY(GXTexture* normalTexture READ normalTexture WRITE setNormalTexture NOTIFY normalTextureChanged)
    Q_PROPERTY(float normalScale READ normalScale WRITE setNormalScale NOTIFY normalScaleChanged)
    Q_PROPERTY(float metallic READ metallic WRITE setMetallic NOTIFY metallicChanged)
    Q_PROPERTY(float roughness READ roughness WRITE setRoughness NOTIFY roughnessChanged)

    Q_PROPERTY(float fresnelBias READ fresnelBias WRITE setFresnelBias NOTIFY fresnelBiasChanged)
    Q_PROPERTY(float fresnelPower READ fresnelPower WRITE setFresnelPower NOTIFY fresnelPowerChanged)
    Q_PROPERTY(float fresnelScale READ fresnelScale WRITE setFresnelScale NOTIFY fresnelScaleChanged)

    Q_PROPERTY(float specularAmount READ specularAmount WRITE setSpecularAmount NOTIFY specularAmountChanged)
    Q_PROPERTY(SpecularChannel specularChannel READ specularChannel WRITE setSpecularChannel NOTIFY specularChannelChanged)

public:
    enum EmissionLight {
        None,
        FakeLight,
        TrueLight
    };
    Q_ENUM(EmissionLight)

    explicit GXPrincipledMaterial(QObject* parent = nullptr);

    QShader vertexShader() const override;
    QShader fragmentShader() const override;
    void applyTo(QRhiGraphicsPipeline* ps) const override;

    int vsBinding() const override { return Principled_VS_Binding; }
    int fsBinding() const override { return Principled_FS_Binding; }
    int vsUboSize() const override { return int(sizeof(PrincipledVSUBO)); }
    int fsUboSize() const override { return int(sizeof(PrincipledFSUBO)); }

    void fillVS(void* dst, const QMatrix4x4& mvp, const QMatrix4x4& model) const override;
    void fillFS(void* dst) const override;

    void ensureBaseColorResources(QRhi* rhi, QRhiCommandBuffer* cb) override;
    void ensureNormalMapResources(QRhi* rhi, QRhiCommandBuffer* cb) override;

    quint32 variantKey() const override;

    QColor baseColor() const { return m_baseColor; }
    void setBaseColor(const QColor& c);

    GXTexture* baseColorTexture() const { return m_baseColorTexture; }
    void setBaseColorTexture(GXTexture* tex);

    QColor emissionColor() const { return m_emissionColor; }
    void setEmissionColor(const QColor& c);

    float emissionStrength() const { return m_emissionStrength; }
    void setEmissionStrength(float s);

    EmissionLight emissionLight() const { return m_emissionLight; }
    void setEmissionLight(const EmissionLight& l);

    float emissionLightIntensity() const { return m_emissionLightIntensity; }
    void setEmissionLightIntensity(float l);

    float emissionLightRadius() const { return m_emissionLightRadius; }
    void setEmissionLightRadius(float l);

    GXTexture* normalTexture() const { return m_normalTexure; }
    void setNormalTexture(GXTexture* tex);

    float normalScale() const { return m_normalScale; }
    void setNormalScale(float s);

    float metallic() const { return m_metallic; }
    void setMetallic(float m);

    float roughness() const { return m_roughness; }
    void setRoughness(float r);

    float fresnelBias() const { return m_fresnelBias; }
    void setFresnelBias(float fb);

    float fresnelPower() const { return m_fresnelPower; }
    void setFresnelPower(float fp);

    float fresnelScale() const { return m_fresnelScale; }
    void setFresnelScale(float fs);

    float specularAmount() const { return m_specularAmount; }
    void setSpecularAmount(float sa);

    SpecularChannel specularChannel() const { return m_specularChannel; }
    void setSpecularChannel(SpecularChannel r);

signals:
    void baseColorChanged();
    void baseColorTextureChanged();
    void emissionColorChanged();
    void emissionStrengthChanged();
    void emissionLightChanged();
    void emissionLightIntensityChanged();
    void emissionLightRadiusChanged();
    void normalTextureChanged();
    void normalScaleChanged();
    void metallicChanged();
    void roughnessChanged();
    void fresnelBiasChanged();
    void fresnelPowerChanged();
    void fresnelScaleChanged();
    void specularAmountChanged();
    void specularChannelChanged();

private:
    QShader m_vs, m_fs;
    QColor m_baseColor = Qt::white;
    GXTexture* m_baseColorTexture = nullptr;
    GXTexture* m_normalTexure = nullptr;
    QColor m_emissionColor = Qt::black;

    float m_emissionStrength = 0.0f;
    float m_emissionLightIntensity = 0.0f;
    float m_emissionLightRadius = 0.0f;
    float m_normalScale = 0.0f;
    float m_metallic = 0.0f;
    float m_roughness = 0.5f;
    float m_fresnelBias = 0.02f;
    float m_fresnelPower = 4.0f;
    float m_fresnelScale = 0.15f;
    float m_specularAmount = 0.25f;
    SpecularChannel m_specularChannel = R;

    bool m_textureMap = false;
    bool m_normalMap = false;

    EmissionLight m_emissionLight = None;

    GXTexture2D* m_solidColorTex = nullptr;
    GXTexture2D* m_solidNormalTex = nullptr;
};

}

#endif // GXPRINCIPLEDMATERIAL_H
