// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <QtQml/qqml.h>
#include <QQmlEngine>

#include <GenesisX/Notifications/NotificationHandler.h>
#include "NotificationsQml.h"

using namespace gx::app::notifications;

void registerGenesisXNotifications(QQmlEngine* engine)
{
    Q_UNUSED(engine);

    qmlRegisterType<gx::app::notifications::NotificationHandler>("GenesisX.Notifications", 1, 0, "NotificationHandler");
}
