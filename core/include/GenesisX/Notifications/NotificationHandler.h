// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef NOTIFICATION_HANDLER_H
#define NOTIFICATION_HANDLER_H

#include <GenesisX/genesisx_global.h>

#include <QtQml/qqmlregistration.h>
#include <QObject>
#include <QTimer>
#include <QIcon>
#include <QVariantMap>

class QSystemTrayIcon;

namespace gx::app::notifications {

class GENESISX_CORE_EXPORT NotificationHandler : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool initialized READ initialized NOTIFY initializedChanged)

public:
    explicit NotificationHandler(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE void initialize(const QVariantMap& options = {});
    Q_INVOKABLE void setIcon(QString icon);
    Q_INVOKABLE void show(const QString& title, const QString& body, int msec = 5000);
    Q_INVOKABLE void subscribe(const QString& topic);
    Q_INVOKABLE void unsubscribe(const QString& topic);
    Q_INVOKABLE QString fcmToken() const;

    bool initialized() const;

    void appleDidReceiveToken(const QString& token);
    void appleDidReceiveRemote(const QString& title, const QString&body, const QVariantMap&data = {});

signals:
    /*!
        \qmlsignal NotificationHandler::initializedChanged()

        Emitted whenever the \l initialised property changes.
     */
    void initializedChanged();
    /*!
        \qmlsignal NotificationHandler::notificationReceived(string title,
                                                             string body,
                                                             var data)

        Emitted when a notification is received.

        The \a title and \a body contain the notification text.
        The \a data map may include extra key–value pairs from the payload.
     */
    void notificationReceived(const QString& title, const QString& body, const QVariantMap& data);
    /*!
        \qmlsignal NotificationHandler::tokenChanged(string token)

        Emitted when the push-notification token changes.
        The new token is provided in \a token.
     */
    void tokenChanged(const QString& token);

private:
    bool m_initialized = false;
    QIcon icon;

#ifdef Q_OS_WIN
    QSystemTrayIcon* m_tray = nullptr;

    /*!
        Ensures that the Windows system tray icon exists and is visible.
     */
    void ensureTray();
    /*!
        Bool check icon is present
     */
    bool eventFilter(QObject*, QEvent* e);
#endif

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    // Keep last id so a new notification can replace the previous one (optional)
    uint m_linuxReplacesId = 0;
    void showLinuxNotification(const QString& title, const QString& body, int msec);
#endif

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    void appleInitialize(const QVariantMap& options);
    void showAppleNotification(const QString& title, const QString& body, int msec);
#endif
};

}

#endif // NOTIFICATION_HANDLER_H

