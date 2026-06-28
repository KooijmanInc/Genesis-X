// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXCAMERA_H
#define GXCAMERA_H

#include <QObject>
#include <QVector3D>
#include <QMatrix4x4>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>
#include <GenesisX/GX3D/Scene/Nodes/GXNode.h>

namespace gx::gx3d {

class GENESISX_GX3D_EXPORT GXCamera : public scene::GXNode
{
    Q_OBJECT

    Q_PROPERTY(QVector3D lookAt READ lookAt WRITE setLookAt NOTIFY lookAtChanged)
    Q_PROPERTY(QVector3D up READ up WRITE setUp NOTIFY upChanged)

public:
    explicit GXCamera(QObject* parent = nullptr);

    QVector3D lookAt() const { return m_lookAt; }
    void setLookAt(const QVector3D& v);

    QVector3D up() const { return m_up; }
    void setUp(const QVector3D& v) { if (m_up == v) return; m_up = v; emit upChanged(); }

    void clearLookAt();

    QMatrix4x4 viewMatrix() const;

    virtual QMatrix4x4 projectionMatrix(float aspect) const = 0;

signals:
    void lookAtChanged();
    void upChanged();
    void changed();

protected:
    QVector3D m_lookAt { 1024.0f, 1024.0f, 1024.0f };
    QVector3D m_up { 0.0f, 1.0f, 0.0f };

    bool m_useLookAt = false;
};

}

#endif // GXCAMERA_H
