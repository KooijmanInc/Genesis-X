// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "CommandControllerQml.h"
#include <QQmlEngine>

#include "include/GenesisX/Orm/CommandController.h"

namespace gx::orm {

void registerGenesisXCommandController(QQmlEngine *engine)
{
    Q_UNUSED(engine);

    qmlRegisterType<gx::orm::CommandController>("GenesisX.Orm.CommandController", 1, 0, "CommandController");
}

}
