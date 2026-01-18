// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/GX3D/genesisx_gx3d_global.h>
#include <GenesisX/GX3D/GX3DQml.h>
#include <GenesisX/GX3D/GXView3D.h>

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

    qmlRegisterType<gx::gx3d::GXView3D>("GenesisX.GX3D", 1, 0, "GXView3D");
}

}
