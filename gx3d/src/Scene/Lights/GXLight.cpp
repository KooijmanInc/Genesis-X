// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Scene/Lights/GXLight.h>

using namespace gx::gx3d::scene;

// static inline bool fuzzyVec3(const QVector3D& a, const QVector3D& b, float eps = 1e-4)
// {
//     return (a - b).lengthSquared() <= eps * eps;
// }

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

// void GXLight::setRotation(const QQuaternion &rot)
// {
//     if (m_rotation == rot) return;
//     m_rotation = rot;

//     m_eulerRotation = m_rotation.toEulerAngles();

//     emit rotationChanged();
//     emit eulerRotationChanged();
// }

// void GXLight::setEulerRotation(const QVector3D &eRot)
// {
//     if (fuzzyVec3(eRot, m_eulerRotation)) return;

//     m_eulerRotation = eRot;
//     m_rotation = QQuaternion::fromEulerAngles(m_eulerRotation);

//     emit eulerRotationChanged();
//     emit rotationChanged();
// }
