// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Scene/Lights/GXDirectionalLight.h>

namespace gx::gx3d::scene {

GXDirectionalLight::GXDirectionalLight(QObject *parent)
    : GXLight{parent}
{

}

void GXDirectionalLight::setDirection(const QVector3D& d)
{
    if (d.isNull()) return;
    m_direction = d.normalized();
    emit directionChanged();
}

void GXDirectionalLight::setIntensity(float i)
{
    m_intensity = qMax(0.0f, i);
    emit intensityChanged();
}

void GXDirectionalLight::setColor(const QColor& c)
{
    m_color = c;
    emit colorChanged();
}

}
