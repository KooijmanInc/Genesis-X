// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXPRINCIPLEDMATERIAL_H
#define GXPRINCIPLEDMATERIAL_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Render/Materials/GXMaterial.h>
#include <GenesisX/GX3D/Render/Materials/GXPrincipledUniforms.h>

namespace gx::gx3d::render {

class GENESISX_GX3D_EXPORT GXPrincipledMaterial : public GXMaterial
{
    Q_OBJECT

    Q_PROPERTY(QColor baseColor READ baseColor WRITE setBaseColor NOTIFY baseColorChanged)
    Q_PROPERTY(QColor emissiveColor READ emissiveColor WRITE setEmissiveColor NOTIFY emissiveColorChanged)
    Q_PROPERTY(float emissiveStrength READ emissiveStrength WRITE setEmissiveStrength NOTIFY emissiveStrengthChanged)
    Q_PROPERTY(bool emissiveLight READ emissiveLight WRITE setEmissiveLight NOTIFY emissiveLightChanged)
    Q_PROPERTY(float emissiveLightIntensity READ emissiveLightIntensity WRITE setEmissiveLightIntensity NOTIFY emissiveLightIntensityChanged)
    Q_PROPERTY(float emissiveLightRadius READ emissiveLightRadius WRITE setEmissiveLightRadius NOTIFY emissiveLightRadiusChanged)

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

    QColor baseColor() const { return m_baseColor; }
    void setBaseColor(const QColor& c);

    QColor emissiveColor() const { return m_emissiveColor; }
    void setEmissiveColor(const QColor& c);

    float emissiveStrength() const { return m_emissiveStrength; }
    void setEmissiveStrength(float s);

    bool emissiveLight() const { return m_emissiveLight; }
    void setEmissiveLight(bool l);

    float emissiveLightIntensity() const { return m_emissiveLightIntensity; }
    void setEmissiveLightIntensity(float l);

    float emissiveLightRadius() const { return m_emissiveLightRadius; }
    void setEmissiveLightRadius(float l);

signals:
    void baseColorChanged();
    void emissiveColorChanged();
    void emissiveStrengthChanged();
    void emissiveLightChanged();
    void emissiveLightIntensityChanged();
    void emissiveLightRadiusChanged();

private:
    QColor m_baseColor = Qt::white;
    QColor m_emissiveColor = Qt::black;

    float m_emissiveStrength = 0.0f;
    float m_emissiveLightIntensity = 0.0f;
    float m_emissiveLightRadius = 0.0f;

    bool m_emissiveLight = false;
};

}

#endif // GXPRINCIPLEDMATERIAL_H
