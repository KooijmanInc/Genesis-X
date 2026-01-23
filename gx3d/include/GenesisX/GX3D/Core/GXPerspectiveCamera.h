// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXPERSPECTIVECAMERA_H
#define GXPERSPECTIVECAMERA_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>
#include <GenesisX/GX3D/Core/GXCamera.h>

#include <QtGlobal>

namespace gx::gx3d {

class GENESISX_GX3D_EXPORT GXPerspectiveCamera : public GXCamera
{
    Q_OBJECT

    Q_PROPERTY(float fov READ fov WRITE setFov NOTIFY fovChanged)
    Q_PROPERTY(float nearPlane READ nearPlane WRITE setNearPlane NOTIFY nearPlaneChanged)
    Q_PROPERTY(float farPlane READ farPlane WRITE setFarPlane NOTIFY farPlaneChanged)

public:
    explicit GXPerspectiveCamera(QObject* parent = nullptr);

    float fov() const { return m_fov; }
    void setFov(float v);

    float nearPlane() const { return m_near; }
    void setNearPlane(float v);

    float farPlane() const { return m_far; }
    void setFarPlane(float v);

    QMatrix4x4 projectionMatrix(float aspect) const override
    {
        if (aspect <= 0.0f) aspect = 1.0f;
        QMatrix4x4 p;
        p.perspective(m_fov, aspect, m_near, m_far);
        return p;
    }

signals:
    void fovChanged();
    void nearPlaneChanged();
    void farPlaneChanged();

private:
    float m_fov = 60.0f;
    float m_near = 0.1f;
    float m_far = 1000.0f;
};

}

#endif // GXPERSPECTIVECAMERA_H
