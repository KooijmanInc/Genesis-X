// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef WEBSITEQML_H
#define WEBSITEQML_H

#include <GenesisX/Web/genesisx_web_global.h>

class QQmlEngine;

namespace gx::web {

GENESISX_WEB_EXPORT void registerGenesisXWebsite(QQmlEngine* engine);

}

#endif // WEBSITEQML_H
