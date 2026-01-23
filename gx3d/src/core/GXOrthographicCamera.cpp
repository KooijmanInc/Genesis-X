// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Core/GXOrthographicCamera.h>

using namespace gx::gx3d;

GXOrthographicCamera::GXOrthographicCamera(QObject *parent)
    : GXCamera{parent}
{
}

void GXOrthographicCamera::setLeft(float v)
{
    if (qFuzzyCompare(m_left, v)) return;
    m_left = v;

    emit leftChanged();
}

void GXOrthographicCamera::setRight(float v)
{
    if (qFuzzyCompare(m_right, v)) return;
    m_right = v;

    emit rightChanged();
}

void GXOrthographicCamera::setBottom(float v)
{
    if (qFuzzyCompare(m_bottom, v)) return;
    m_bottom = v;

    emit bottomChanged();
}

void GXOrthographicCamera::setTop(float v)
{
    if (qFuzzyCompare(m_top, v)) return;
    m_top = v;

    emit topChanged();
}

void GXOrthographicCamera::setNearPlane(float v)
{
    if (qFuzzyCompare(m_near, v)) return;
    m_near = v;

    emit nearPlaneChanged();
}

void GXOrthographicCamera::setFarPlane(float v)
{
    if (qFuzzyCompare(m_far, v)) return;
    m_far = v;

    emit farPlaneChanged();
}
