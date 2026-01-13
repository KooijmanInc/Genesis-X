// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/utils/SettingsManager.h>
#include "SettingsManagerQml.h"

#include <GenesisX/utils/SettingsManager.h>

using namespace gx::utils;

void registerGenesisXSettingsManager(QQmlEngine *engine)
{
    Q_UNUSED(engine);

    qmlRegisterSingletonType<gx::utils::SettingsManager>("GenesisX.System", 1, 0, "Settings", [](QQmlEngine*, QJSEngine*) -> QObject* { return new SettingsManager; });
}
