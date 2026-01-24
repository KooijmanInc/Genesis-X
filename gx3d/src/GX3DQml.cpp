// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/genesisx_gx3d_global.h>
#include <GenesisX/GX3D/GX3DQml.h>
#include <GenesisX/GX3D/QtQuick/GXView3D.h>
#include <GenesisX/GX3D/Core/GXCamera.h>
#include <GenesisX/GX3D/Core/GXPerspectiveCamera.h>
#include <GenesisX/GX3D/Core/GXOrthographicCamera.h>
#include <GenesisX/GX3D/Scene/Nodes/GXNode.h>
#include <GenesisX/GX3D/Scene/Lights/GXPointLight.h>
#include <GenesisX/GX3D/Render/Nodes/GXModelNode.h>
#include <GenesisX/GX3D/Render/Nodes/GXRenderableNode.h>
#include <GenesisX/GX3D/Scene/GXScene.h>
#include <GenesisX/GX3D/Render/Materials/GXMaterial.h>
#include <GenesisX/GX3D/Render/Materials/GXDefaultLitMaterial.h>
#include <GenesisX/GX3D/Render/Materials/GXPrincipledMaterial.h>

#include <QQmlEngine>

/*!
    \namespace gx::gx3d
    \inmodule GenesisX
    \title gx::gx3d Namespace
    \brief GX3D facilities.
 */
namespace gx::gx3d {

void registerEnabledQmlModules(QQmlEngine *engine)
{
    Q_UNUSED(engine);

    qmlRegisterType<gx::gx3d::GXView3D>("GenesisX3D", 1, 0, "GXView3D");

    qmlRegisterUncreatableType<gx::gx3d::GXCamera>("GenesisX3D", 1, 0, "GXCamera", "GXCamera is an abstract base class. Use GXPerspectiveCamera or GXOrthographicCamera.");
    qmlRegisterUncreatableType<gx::gx3d::render::GXRenderableNode>("GenesisX3D", 1, 0, "GXRenderableNode", "GXRenderableNode is an abstract base class. Use GXNode.");
    qmlRegisterUncreatableType<gx::gx3d::render::GXMaterial>("GenesisX3D", 1, 0, "GXMaterial", "GXMaterial is an abstract base class. Use GXDefaultLitMaterial or GXPrincipledMaterial");

    qmlRegisterType<gx::gx3d::GXPerspectiveCamera>("GenesisX3D", 1, 0, "GXPerspectiveCamera");
    qmlRegisterType<gx::gx3d::GXOrthographicCamera>("GenesisX3D", 1, 0, "GXOrthographicCamera");

    qmlRegisterType<gx::gx3d::scene::GXNode>("GenesisX3D", 1, 0, "GXNode");
    qmlRegisterType<gx::gx3d::scene::GXScene>("GenesisX3D", 1, 0, "GXScene");
    qmlRegisterType<gx::gx3d::scene::GXPointLight>("GenesisX3D", 1, 0, "GXPointLight");

    qmlRegisterType<gx::gx3d::render::GXModel>("GenesisX3D", 1, 0, "GXModel");

    qRegisterMetaType<gx::gx3d::render::GXMaterial>("gx::gx3d::render::GXMaterial*");
    qmlRegisterType<gx::gx3d::render::GXDefaultLitMaterial>("GenesisX3D", 1, 0, "GXDefaultLitMaterial");
    qmlRegisterType<gx::gx3d::render::GXPrincipledMaterial>("GenesisX3D", 1, 0, "GXPrincipledMaterial");
}

}
