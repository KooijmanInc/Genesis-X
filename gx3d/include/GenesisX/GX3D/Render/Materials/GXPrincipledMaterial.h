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
    Q_PROPERTY(bool emissionLight READ emissionLight WRITE setEmissionLight NOTIFY emissionLightChanged)
    Q_PROPERTY(float emissionLightIntensity READ emissionLightIntensity WRITE setEmissionLightIntensity NOTIFY emissionLightIntensityChanged)
    Q_PROPERTY(float emissionLightRadius READ emissionLightRadius WRITE setEmissionLightRadius NOTIFY emissionLightRadiusChanged)

public:
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

    QColor baseColor() const { return m_baseColor; }
    void setBaseColor(const QColor& c);

    GXTexture* baseColorTexture() const { return m_baseColorTexture; }
    void setBaseColorTexture(GXTexture* tex);

    QColor emissionColor() const { return m_emissionColor; }
    void setEmissionColor(const QColor& c);

    float emissionStrength() const { return m_emissionStrength; }
    void setEmissionStrength(float s);

    bool emissionLight() const { return m_emissionLight; }
    void setEmissionLight(bool l);

    float emissionLightIntensity() const { return m_emissionLightIntensity; }
    void setEmissionLightIntensity(float l);

    float emissionLightRadius() const { return m_emissionLightRadius; }
    void setEmissionLightRadius(float l);

signals:
    void baseColorChanged();
    void baseColorTextureChanged();
    void emissionColorChanged();
    void emissionStrengthChanged();
    void emissionLightChanged();
    void emissionLightIntensityChanged();
    void emissionLightRadiusChanged();

private:
    QShader m_vs, m_fs;
    QColor m_baseColor = Qt::white;
    GXTexture* m_baseColorTexture = nullptr;
    QColor m_emissionColor = Qt::black;

    float m_emissionStrength = 0.0f;
    float m_emissionLightIntensity = 0.0f;
    float m_emissionLightRadius = 0.0f;

    bool m_emissionLight = false;

    GXTexture2D* m_solidColorTex = nullptr;
};

}

#endif // GXPRINCIPLEDMATERIAL_H
