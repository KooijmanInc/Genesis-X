// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <QCoreApplication>
#include <QQmlEngine>
#include <QtQml/qqml.h>
#include "NotificationHandler.h"
#include <GenesisX/NotificationsQml.h>

namespace gx {

// void registerNotificationsQmlTypes(const char* uri, int maj, int min)
// {
//     qmlRegisterType<gx::NotificationHandler>(uri, maj, min, "NotificationHandler");
// }
void registerGenesisXNotifications() {
    qmlRegisterType<gx::NotificationHandler>("GenesisX.Notifications", 1, 0, "NotificationHandler");
}


#ifdef GX_ENABLE_STARTUP_AUTO_REGISTER
static void _autoRegister() { registerGenesisXNotifications(); }
Q_COREAPP_STARTUP_FUNCTION(_autoRegister)
#endif
}
// #if defined(GENESISX_STATIC) || defined(QT_STATIC)
static void initQrc() { Q_INIT_RESOURCE(core); }
Q_COREAPP_STARTUP_FUNCTION(initQrc)
// #endif

