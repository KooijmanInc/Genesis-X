// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Core/GXCamera.h>

using namespace gx::gx3d;

GXCamera::GXCamera(QObject *parent)
    : QObject{parent}
{
}

void GXCamera::setTarget(const QVector3D &v)
{
    if (m_target == v) return;
    m_target = v;
    m_useTarget = true;

    emit targetChanged();
}

void GXCamera::clearTarget()
{
    m_useTarget = false;

    emit targetChanged();
}

QMatrix4x4 GXCamera::viewMatrix() const
{
    QMatrix4x4 v;

    if (m_useTarget) {
        v.lookAt(m_position, m_target, m_up);
    } else {
        // QVector3D forward(0, 0, -1);
        v.translate(-m_position);
    }

    return v;
}
