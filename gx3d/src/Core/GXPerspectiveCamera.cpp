// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Core/GXPerspectiveCamera.h>

using namespace gx::gx3d;

GXPerspectiveCamera::GXPerspectiveCamera(QObject *parent)
    : GXCamera{parent}
{
}

void GXPerspectiveCamera::setFov(float v)
{
    if (qFuzzyCompare(m_fov, v)) return;
    m_fov = v;

    emit fovChanged();
}

void GXPerspectiveCamera::setNearPlane(float v)
{
    if (qFuzzyCompare(m_near, v)) return;
    m_near = v;

    emit nearPlaneChanged();
}

void GXPerspectiveCamera::setFarPlane(float v)
{
    if (qFuzzyCompare(m_far, v)) return;
    m_far = v;

    emit farPlaneChanged();
}
