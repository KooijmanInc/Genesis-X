// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/OrmQml.h>

#include <QQmlEngine>

#include "src/core/ConnectionControllerQml.h"
#include "src/core/CommandControllerQml.h"

/*!
    \namespace gx::orm
    \inmodule GenesisX
    \title gx::orm Namespace
    \brief Orm facilities.
 */

namespace gx::orm {

void registerEnabledQmlModules(QQmlEngine* engine)
{
    registerGenesisXConnectionController(engine);
    registerGenesisXCommandController(engine);
}

}
