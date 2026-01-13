// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Framework/genesisx_framework_global.h>
#include <GenesisX/Framework/FrameworkQml.h>
#include <GenesisX/Framework/GenericJsonModel.h>
#include <GenesisX/Framework/DeviceControl/OrientationController.h>
#include <GenesisX/Framework/DeviceControl/KeepAwake.h>

#include <QQmlEngine>

/*!
    \namespace gx::framework
    \inmodule GenesisX
    \title gx::framework Namespace
    \brief Framework facilities.
 */

namespace gx::framework {

static devicecontrol::OrientationController* s_orientation = nullptr;

void registerEnabledQmlModules(QQmlEngine *engine)
{
    Q_UNUSED(engine);

    if (!s_orientation) s_orientation = new devicecontrol::OrientationController();

    qmlRegisterType<gx::framework::GenericJsonModel>("GenesisX.Framework", 1, 0, "GenericJsonModel");
    qmlRegisterType<gx::framework::KeepAwake>("GenesisX.Framework", 1, 0, "KeepAwake");
    qmlRegisterSingletonInstance("GenesisX.Framework", 1, 0, "OrientationController", s_orientation);
}

}
