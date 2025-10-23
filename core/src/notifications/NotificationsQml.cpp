// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <QCoreApplication>
#include <QQmlEngine>
#include <QtQml/qqml.h>
#include "NotificationHandler.h"
#include <GenesisX/NotificationsQml.h>

namespace gx {

void registerGenesisXNotifications() {
    qmlRegisterType<gx::NotificationHandler>("GenesisX", 1, 0, "NotificationHandler");

}


static void _autoRegister() { registerGenesisXNotifications(); }
Q_COREAPP_STARTUP_FUNCTION(_autoRegister)
}

// static void initQrc() { Q_INIT_RESOURCE(core); }
// Q_COREAPP_STARTUP_FUNCTION(initQrc)
