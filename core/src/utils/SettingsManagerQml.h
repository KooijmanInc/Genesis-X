// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef SETTINGSMANAGERQML_H
#define SETTINGSMANAGERQML_H

#include <QtQml/qqml.h>

#include <GenesisX/genesisx_global.h>

class QQmlEngine;

namespace gx::utils {

GENESISX_CORE_EXPORT void registerGenesisXSettingsManager(QQmlEngine* engine);

}


#endif // SETTINGSMANAGERQML_H
