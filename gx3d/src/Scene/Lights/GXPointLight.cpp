// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Scene/Lights/GXPointLight.h>

using namespace gx::gx3d::scene;

GXPointLight::GXPointLight(QObject *parent)
    : GXLight{parent}
{
}

void GXPointLight::setRange(float r)
{
    if (qFuzzyCompare(m_range, r)) return;
    m_range = r;

    emit rangeChanged();
}
