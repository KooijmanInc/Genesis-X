// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Core/GXCamera.h>

using namespace gx::gx3d;

GXCamera::GXCamera(QObject *parent)
    : scene::GXNode{parent}
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
    if (m_useTarget) {
        QMatrix4x4 v;
        v.setToIdentity();
        v.lookAt(worldPosition(), m_target, m_up);
        return v;
    }

    // Free camera: inverse of node transform
    return worldMatrix().inverted();
}
