// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Core/GXCamera.h>

using namespace gx::gx3d;

GXCamera::GXCamera(QObject *parent)
    : scene::GXNode{parent}
{
}

void GXCamera::setLookAt(const QVector3D &v)
{//qDebug() << "default = " << m_lookAt;
    if (m_lookAt == v) return;
    m_lookAt = v;
    m_useLookAt = true;
    //qDebug() << "useLookAt in setLookAt" << m_useLookAt;

    emit lookAtChanged();
}

void GXCamera::clearLookAt()
{//qDebug() << "clearLookAt activated??";
    m_useLookAt = false;

    emit lookAtChanged();
}

QMatrix4x4 GXCamera::viewMatrix() const
{//qDebug() << "useLookAt" << m_useLookAt;
    if (m_useLookAt) {
        QMatrix4x4 v;
        v.setToIdentity();
        v.lookAt(worldPosition(), m_lookAt, m_up);
        return v;
    }

    // Free camera: inverse of node transform
    return worldMatrix().inverted();
}
