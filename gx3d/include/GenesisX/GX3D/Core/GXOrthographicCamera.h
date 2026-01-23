// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXORTHOGRAPHICCAMERA_H
#define GXORTHOGRAPHICCAMERA_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>
#include <GenesisX/GX3D/Core/GXCamera.h>

#include <QtGlobal>

namespace gx::gx3d {

class GENESISX_GX3D_EXPORT GXOrthographicCamera : public GXCamera
{
    Q_OBJECT

    Q_PROPERTY(float left READ left WRITE setLeft NOTIFY leftChanged)
    Q_PROPERTY(float right READ right WRITE setRight NOTIFY rightChanged)
    Q_PROPERTY(float bottom READ bottom WRITE setBottom NOTIFY bottomChanged)
    Q_PROPERTY(float top READ top WRITE setTop NOTIFY topChanged)
    Q_PROPERTY(float nearPlane READ nearPlane WRITE setNearPlane NOTIFY nearPlaneChanged)
    Q_PROPERTY(float farPlane READ farPlane WRITE setFarPlane NOTIFY farPlaneChanged)

public:
    explicit GXOrthographicCamera(QObject *parent = nullptr);

    float left() const { return m_left; }
    void setLeft(float v);

    float right() const { return m_right; }
    void setRight(float v);

    float bottom() const { return m_bottom; }
    void setBottom(float v);

    float top() const { return m_top; }
    void setTop(float v);

    float nearPlane() const { return m_near; }
    void setNearPlane(float v);

    float farPlane() const { return m_far; }
    void setFarPlane(float v);

    QMatrix4x4 projectionMatrix(float /*aspect*/) const override
    {
        QMatrix4x4 p;
        p.ortho(m_left, m_right, m_bottom, m_top, m_near, m_far);
        return p;
    }

signals:
    void leftChanged();
    void rightChanged();
    void bottomChanged();
    void topChanged();
    void nearPlaneChanged();
    void farPlaneChanged();

private:
    float m_left = -1.0f;
    float m_right = 1.0f;
    float m_bottom = -1.0f;
    float m_top = 1.0f;
    float m_near = 0.1f;
    float m_far = 1000.0f;
};

}

#endif // GXORTHOGRAPHICCAMERA_H
