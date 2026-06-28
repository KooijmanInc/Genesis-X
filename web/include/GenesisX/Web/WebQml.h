// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef WEBQML_H
#define WEBQML_H

#include <QString>

#include "genesisx_web_global.h"

class QQmlEngine;

namespace gx::web {

GENESISX_WEB_EXPORT void registerEnabledQmlModules(QQmlEngine* engine);

}

#endif // WEBQML_H
