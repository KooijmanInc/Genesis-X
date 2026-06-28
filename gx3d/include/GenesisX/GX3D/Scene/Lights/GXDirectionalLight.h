// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXDIRECTIONALLIGHT_H
#define GXDIRECTIONALLIGHT_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Scene/Lights/GXLight.h>

namespace gx::gx3d::scene {

class GXDirectionalLight : public scene::GXLight
{
    Q_OBJECT
    Q_PROPERTY(QVector3D direction READ direction WRITE setDirection NOTIFY directionChanged)
    Q_PROPERTY(float intensity READ intensity WRITE setIntensity NOTIFY intensityChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    explicit GXDirectionalLight(QObject* parent = nullptr);

    QVector3D direction() const { return m_direction; }
    void setDirection(const QVector3D& d);

    float intensity() const { return m_intensity; }
    void setIntensity(float i);

    QColor color() const { return m_color; }
    void setColor(const QColor& c);

signals:
    void directionChanged();
    void intensityChanged();
    void colorChanged();

private:
    QVector3D m_direction = QVector3D(0, -1, -1).normalized();
    float     m_intensity = 1.0f;
    QColor    m_color     = Qt::white;
};


}

#endif // GXDIRECTIONALLIGHT_H
