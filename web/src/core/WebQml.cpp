// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Web/WebQml.h>

#include <QQmlEngine>

#include "src/core/WebsiteQml.h"

/*!
    \namespace gx::web
    \inmodule GenesisX
    \title gx::web Namespace
    \brief Web facilities.
 */

namespace gx::web {

void registerEnabledQmlModules(QQmlEngine *engine)
{
    registerGenesisXWebsite(engine);
}

}
