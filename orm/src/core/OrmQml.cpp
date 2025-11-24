// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "include/GenesisX/Orm/OrmQml.h"

#include <QQmlEngine>

#include "src/core/CommandControllerQml.h"

/*!
    \namespace gx::orm
    \inmodule GenesisX
    \title gx::orm Namespace
    \brief Orm facilities.
 */

namespace gx::orm {

void registerGenesisXCommandController(QQmlEngine*);

void registerEnabledQmlModules(QQmlEngine* engine)
{
    registerGenesisXCommandController(engine);
}

}
