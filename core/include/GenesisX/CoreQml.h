// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef COREQML_H
#define COREQML_H

#include <QString>

#include "genesisx_global.h"

class QQmlEngine;

namespace gx::core {

GENESISX_CORE_EXPORT void registerEnabledQmlModules(QQmlEngine* engine, QString features = "");

}

#endif // COREQML_H
