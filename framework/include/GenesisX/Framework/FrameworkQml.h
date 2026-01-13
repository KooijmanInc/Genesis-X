// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef FRAMEWORKQML_H
#define FRAMEWORKQML_H

#include "genesisx_framework_global.h"

class QQmlEngine;

namespace gx::framework {

GENESISX_FRAMEWORK_EXPORT void registerEnabledQmlModules(QQmlEngine* engine);

}

#endif // FRAMEWORKQML_H
