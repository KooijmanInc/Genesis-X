// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXLIGHT_H
#define GXLIGHT_H

#include <QColor>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Scene/Nodes/GXNode.h>

namespace gx::gx3d::scene {

class GENESISX_GX3D_EXPORT GXLight : public GXNode
{
    Q_OBJECT

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(float intensity READ intensity WRITE setIntensity NOTIFY intensityChanged)

public:
    explicit GXLight(QObject* parent = nullptr);

    QColor color() const { return m_color; }
    void setColor(const QColor& c);

    float intensity() const { return m_intensity; }
    void setIntensity(float i);

signals:
    void colorChanged();
    void intensityChanged();

protected:
    QColor m_color = Qt::white;
    float m_intensity = 1.0f;
};

}

#endif // GXLIGHT_H
