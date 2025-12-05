// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef NOTIFICATIONSQML_H
#define NOTIFICATIONSQML_H

#include "include/GenesisX/genesisx_global.h"

class QQmlEngine;

namespace gx::app::notifications {

GENESISX_CORE_EXPORT void registerGenesisXNotifications(QQmlEngine* engine);

}

#endif // NOTIFICATIONSQML_H
