// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXPOINTLIGHT_H
#define GXPOINTLIGHT_H

#include <QVector3D>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

#include <GenesisX/GX3D/Scene/Lights/GXLight.h>

namespace gx::gx3d::scene {

class GENESISX_GX3D_EXPORT GXPointLight : public GXLight
{
    Q_OBJECT

    Q_PROPERTY(float range READ range WRITE setRange NOTIFY rangeChanged)

public:
    explicit GXPointLight(QObject* parent = nullptr);

    float range() const { return m_range; }
    void setRange(float r);

signals:
    void rangeChanged();

private:
    float m_range = 10.0f;
};

}

#endif // GXPOINTLIGHT_H
