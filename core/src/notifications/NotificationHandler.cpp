// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "NotificationHandler.h"
#include <QCoreApplication>

#ifdef Q_OS_ANDROID
#include "fcm_android.h"
#endif

#ifdef Q_OS_WIN
#include <QSystemTrayIcon>
#include <QIcon>
#include <QApplication>
#endif

using namespace gx;


void NotificationHandler::show(const QString &title, const QString &body, int msec)
{
#ifdef Q_OS_WIN
    ensureTray();
    // You must have a visible tray icon for balloons to appear:
    m_tray->showMessage(title, body, qApp->windowIcon(), msec);
#endif
}

void NotificationHandler::initialize(const QVariantMap &options)
{
    Q_UNUSED(options);
    if (m_initialized) return;

#ifdef Q_OS_ANDROID
    auto& fcm = gx::android::FcmBridge::instance();
    connect(&fcm, &gx::android::FcmBridge::messageReceived, this, &NotificationHandler::notificationReceived);
connect(&fcm, &gx::android::FcmBridge::tokenChanged, this, &NotificationHandler::tokenChanged);
    fcm.initialize();
#endif

    m_initialized = true;
    emit initializedChanged();
    qInfo() << "[GX Notify] initialized";
}

void NotificationHandler::subscribe(const QString &topic)
{
#ifdef Q_OS_ANDROID
    qDebug() << "subscribe activated";
    // android::FcmBridge::instance().subscribe(topic);
#else
    qInfo() << "[GX Notify] (stub) subscribe to:" << topic;
#endif
}

void NotificationHandler::unsubscribe(const QString &topic)
{
#ifdef Q_OS_ANDROID
    qDebug() << "unsubscribe activated";
    // android::FcmBridge::instance().unsubscribe(topic);
#else
    qInfo() << "[GX Notify] (stub) unsubscribe from:" << topic;
#endif
}

QString NotificationHandler::fcmToken() const
{
#ifdef Q_OS_ANDROID
    return gx::android::FcmBridge::instance().token();
#else
    return {};
#endif
}
#ifdef Q_OS_WIN
void NotificationHandler::ensureTray()
{
    if (m_tray) return;
    if (!QSystemTrayIcon::isSystemTrayAvailable()) return;

    m_tray = new QSystemTrayIcon(this);

    QIcon icon = qApp->windowIcon();

    if (icon.isNull()) {
        icon = QIcon(":/logo.ico");
    }
    m_tray->setIcon(icon);

    m_tray->setToolTip(qApp->applicationDisplayName().isEmpty()
                           ? QStringLiteral("GenesisX")
                           : qApp->applicationDisplayName());
    m_tray->show();
    qApp->installEventFilter(this);
}

bool NotificationHandler::eventFilter(QObject*, QEvent* e)
{
    if (e->type() == QEvent::ApplicationWindowIconChange && m_tray) {
        QIcon icon = qApp->windowIcon();
        if (!icon.isNull()) m_tray->setIcon(icon);
    }
    return false;
}
#endif

