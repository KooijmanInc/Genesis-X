// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Scene/Lights/GXLight.h>

using namespace gx::gx3d::scene;

GXLight::GXLight(QObject *parent)
    : GXNode{parent}
{
}

void GXLight::setColor(const QColor &c)
{
    if (m_color == c) return;
    m_color = c;

    emit colorChanged();
}

void GXLight::setIntensity(float i)
{
    if (qFuzzyCompare(m_intensity, i)) return;
    m_intensity = i;

    emit intensityChanged();
}
