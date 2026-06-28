// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Render/Environment/GXEnvironment.h>

namespace gx::gx3d::render {

GXEnvironment::GXEnvironment(QObject *parent)
    : QObject{parent}
{
}

void GXEnvironment::setAmbientIntensity(float a)
{
    if (m_ambientIntensity == a) return;
    m_ambientIntensity = qBound(0.0f, a, 1.0f);

    emit ambientIntensityChanged();
}

void GXEnvironment::setAmbientColor(const QColor &c)
{
    if (m_ambientColor == c) return;
    m_ambientColor = c;

    emit ambientColorChanged();
}

void GXEnvironment::setAoStrength(float ao)
{
    if (m_aoStrength == ao) return;
    m_aoStrength = qBound(0.0f, ao, 1.0f);

    emit aoStrengthChanged();
}

void GXEnvironment::setAoRadius(float ao)
{
    if (m_aoRadius == ao) return;
    m_aoRadius = qBound(0.0f, ao, 1.0f);

    emit aoRadiusChanged();
}

void GXEnvironment::setAoSoftness(float ao)
{
    if (m_aoSoftness == ao) return;
    m_aoSoftness = qBound(0.0f, ao, 1.0f);

    emit aoSoftnessChanged();
}

void GXEnvironment::setSkySpecularIntensity(float i)
{
    if (m_skySpecularIntensity == i) return;
    m_skySpecularIntensity = i;

    emit skySpecularIntensityChanged();
}

void GXEnvironment::setSkySpecularDirection(const QVector3D &d)
{
    if (m_skySpecularDirection == d) return;
    m_skySpecularDirection = d;

    emit skySpecularDirectionChanged();
}

void GXEnvironment::setSpecularSource(GXSpecularSource s)
{
    if (m_specularSource == s) return;
    m_specularSource = s;

    emit specularSourceChanged();
}

}
