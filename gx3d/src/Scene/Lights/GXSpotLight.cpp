// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Scene/Lights/GXSpotLight.h>

#include <QtMath>

using namespace gx::gx3d::scene;

GXSpotLight::GXSpotLight(QObject *parent)
    : GXLight{parent}
{

}

// static float clampCone(float rads)
// {
//     const float lo = 0.0f;
//     const float hi = float(M_PI_2);
//     if (rads < lo) return lo;
//     if (rads > hi) return hi;

//     return rads;
// }

static float clampConeDegrees(float deg)
{
    // Prevent 0 (undefined cone) and prevent 180 (cos flips)
    return qBound(0.1f, deg, 89.9f);
}


void GXSpotLight::setInnerConeAngle(float deg)
{
    deg = clampConeDegrees(deg);
    if (deg > m_outerConeAngle) deg = m_outerConeAngle;

    if (qFuzzyCompare(m_innerConeAngle, deg)) return;
    m_innerConeAngle = deg;

    emit innerConeAngleChanged();
    emit changed();
}

void GXSpotLight::setOuterConeAngle(float deg)
{
    deg = clampConeDegrees(deg);
    if (deg < m_innerConeAngle) deg = m_innerConeAngle;

    if (qFuzzyCompare(m_outerConeAngle, deg)) return;
    m_outerConeAngle = deg;

    emit outerConeAngleChanged();
    emit changed();
}

void GXSpotLight::setRange(float r)
{
    if (qFuzzyCompare(m_range, r)) return;
    m_range = r;

    emit rangeChanged();
    emit changed();
}

QVector3D GXSpotLight::directionWS() const
{
    const QMatrix4x4 wm = worldMatrix();
    const QVector3D p = wm.map(QVector3D(0, 0, 0));
    const QVector3D f = wm.map(QVector3D(0, 0, -1));
    QVector3D d = (f - p);
    const float len = d.length();
    if (len > 1e-6f) d /= len;
    else d = QVector3D(0, 0, -1);

    return d;
}
