// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/Scene/Nodes/GXCameraNode.h>

using namespace gx::gx3d;
using namespace gx::gx3d::scene;

GXCameraNode::GXCameraNode(QObject *parent)
    : GXNode{parent}
{
}

void GXCameraNode::setCamera(GXCamera *cam)
{
    if (m_camera == cam) return;
    m_camera = cam;

    emit cameraChanged();
}

QMatrix4x4 GXCameraNode::viewMetrix() const
{
    if (!m_camera) return {};

    QMatrix4x4 v;
    // v.lookAt(
    //     worldPosition(),
    //     m_camera->target(),
    //     m_camera->up()
    // );

    return v;
}

QMatrix4x4 GXCameraNode::projectionMatrix(float aspect) const
{
    if (!m_camera) return {};

    return m_camera->projectionMatrix(aspect);
}
