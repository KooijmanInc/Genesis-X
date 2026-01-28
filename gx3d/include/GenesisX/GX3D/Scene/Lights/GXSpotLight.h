// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXSPOTLIGHT_H
#define GXSPOTLIGHT_H

#include <QtGlobal>
#include <QVector3D>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Scene/Lights/GXLight.h>

namespace gx::gx3d::scene {

class GENESISX_GX3D_EXPORT GXSpotLight : public GXLight
{
    Q_OBJECT

    Q_PROPERTY(float innerConeAngle READ innerConeAngle WRITE setInnerConeAngle NOTIFY innerConeAngleChanged)
    Q_PROPERTY(float outerConeAngle READ outerConeAngle WRITE setOuterConeAngle NOTIFY outerConeAngleChanged)
    Q_PROPERTY(float range READ range WRITE setRange NOTIFY rangeChanged)

public:
    explicit GXSpotLight(QObject* parent = nullptr);

    float innerConeAngle() const { return m_innerConeAngle; }
    void setInnerConeAngle(float rads);

    float outerConeAngle() const { return m_outerConeAngle; }
    void setOuterConeAngle(float rads);

    float range() const { return m_range; }
    void setRange(float r);

    Q_INVOKABLE QVector3D directionWS() const;

signals:
    void innerConeAngleChanged();
    void outerConeAngleChanged();
    void rangeChanged();

private:
    float m_range = 10.0f;
    float m_innerConeAngle = 0.0f;
    float m_outerConeAngle = 0.78539816339f;
};

}

#endif // GXSPOTLIGHT_H
