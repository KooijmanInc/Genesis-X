// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXENVIRONMENT_H
#define GXENVIRONMENT_H

#include <QVector3D>
#include <QObject>
#include <QColor>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

namespace gx::gx3d::render {

class GENESISX_GX3D_EXPORT GXEnvironment : public QObject
{
    Q_OBJECT

    Q_PROPERTY(float ambientIntensity READ ambientIntensity WRITE setAmbientIntensity NOTIFY ambientIntensityChanged)
    Q_PROPERTY(QColor ambientColor READ ambientColor WRITE setAmbientColor NOTIFY ambientColorChanged)

    Q_PROPERTY(float aoStrength READ aoStrength WRITE setAoStrength NOTIFY aoStrengthChanged)
    Q_PROPERTY(float aoRadius READ aoRadius WRITE setAoRadius NOTIFY aoRadiusChanged)
    Q_PROPERTY(float aoSoftness READ aoSoftness WRITE setAoSoftness NOTIFY aoSoftnessChanged)

    Q_PROPERTY(float skySpecularIntensity READ skySpecularIntensity WRITE setSkySpecularIntensity NOTIFY skySpecularIntensityChanged)
    Q_PROPERTY(QVector3D skySpecularDirection READ skySpecularDirection WRITE setSkySpecularDirection NOTIFY skySpecularDirectionChanged)

    Q_PROPERTY(GXSpecularSource specularSource READ specularSource WRITE setSpecularSource NOTIFY specularSourceChanged)

public:
    enum class GXSkyMode {
        None,
        SkyPlane
    };
    Q_ENUM(GXSkyMode)

    enum class GXSpecularSource : quint8 {
        None = 0,
        LightCard = 1,
        SkyGradient = 2,
        SkyBox = 3,
        IBL = 4
    };
    Q_ENUM(GXSpecularSource)

    explicit GXEnvironment(QObject* parent = nullptr);
    ~GXEnvironment() = default;

    float ambientIntensity() const { return m_ambientIntensity; }
    void setAmbientIntensity(float a);
    QColor ambientColor() const { return m_ambientColor; }
    void setAmbientColor(const QColor& c);

    float aoStrength() const { return m_aoStrength; }
    void setAoStrength(float ao);
    float aoRadius() const { return m_aoRadius; }
    void setAoRadius(float ao);
    float aoSoftness() const { return m_aoSoftness; }
    void setAoSoftness(float ao);

    float skySpecularIntensity() const { return m_skySpecularIntensity; }
    void setSkySpecularIntensity(float i);
    QVector3D skySpecularDirection() const { return m_skySpecularDirection; }
    void setSkySpecularDirection(const QVector3D& d);

    GXSpecularSource specularSource() const { return m_specularSource; }
    void setSpecularSource(GXSpecularSource s);

signals:
    void ambientIntensityChanged();
    void ambientColorChanged();

    void aoStrengthChanged();
    void aoRadiusChanged();
    void aoSoftnessChanged();

    void skySpecularIntensityChanged();
    void skySpecularDirectionChanged();

    void specularSourceChanged();

private:
    float m_ambientIntensity = 0.01f;
    QColor m_ambientColor = Qt::white;

    float m_aoStrength = 0.0f;
    float m_aoRadius = 0.0f;
    float m_aoSoftness = 0.0f;

    float m_skySpecularIntensity = 0.35f;
    QVector3D m_skySpecularDirection = {-0.4f,0.7f,0.6f};

    GXSpecularSource m_specularSource = GXSpecularSource::LightCard;
};

}

#endif // GXENVIRONMENT_H
