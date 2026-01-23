// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXCAMERANODE_H
#define GXCAMERANODE_H

#include <GenesisX/GX3D/genesisx_gx3d_global.h>
#include <GenesisX/GX3D/Scene/Nodes/GXNode.h>
#include <GenesisX/GX3D/Core/GXCamera.h>
#include <QMatrix4x4>

namespace gx::gx3d::scene {

class GENESISX_GX3D_EXPORT GXCameraNode : public GXNode
{
    Q_OBJECT

    Q_PROPERTY(gx::gx3d::GXCamera* camera READ camera WRITE setCamera NOTIFY cameraChanged)

public:
    explicit GXCameraNode(QObject* parent = nullptr);

    GXCamera* camera() const { return m_camera; }
    void setCamera(GXCamera* cam);

    QMatrix4x4 viewMetrix() const;
    QMatrix4x4 projectionMatrix(float aspect) const;

signals:
    void cameraChanged();

private:
    GXCamera* m_camera = nullptr;
};

}

#endif // GXCAMERANODE_H
