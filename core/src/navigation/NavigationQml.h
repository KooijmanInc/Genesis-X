// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef NAVIGATIONQML_H
#define NAVIGATIONQML_H

#include "include/GenesisX/genesisx_global.h"

class QQmlEngine;

// #include "GxRouter.h"

namespace gx::navigation {

GENESISX_CORE_EXPORT void registerGenesisXNavigation(QQmlEngine* engine);

// static GxRouter s_router;

// void registerGenesisXNavigation()
// {
//     qmlRegisterSingletonInstance("GenesisX.Core", 1, 0, "Router", &s_router);
// }

// static void _autoRegister() { registerGenesisXNavigation(); }
// Q_COREAPP_STARTUP_FUNCTION(_autoRegister)
}

#endif // NAVIGATIONQML_H
