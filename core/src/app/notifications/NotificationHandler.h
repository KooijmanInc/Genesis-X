// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef NOTIFICATION_HANDLER_H
#define NOTIFICATION_HANDLER_H

#include <GenesisX/genesisx_global.h>

#include <QtQml/qqmlregistration.h>
#include <QObject>
#include <QTimer>
#include <QVariantMap>

class QSystemTrayIcon;

namespace gx::app::notifications {

class GENESISX_CORE_EXPORT NotificationHandler : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool initialized READ initialized NOTIFY initializedChanged)
    // Q_PROPERTY(bool pushSupported READ pushSupported CONSTANT)
    // Q_PROPERTY(bool tokenReady READ tokenReady NOTIFY tokenReadyChanged)

public:
    explicit NotificationHandler(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE void initialize(const QVariantMap& options = {});
    Q_INVOKABLE void show(const QString& title, const QString& body, int msec = 5000);
    Q_INVOKABLE void subscribe(const QString& topic);
    Q_INVOKABLE void unsubscribe(const QString& topic);
    Q_INVOKABLE QString fcmToken() const;

    bool initialized() const { return m_initialized; }
    // bool pushSupported() const { return m_supported; }
    // bool tokenReady() const { return !m_token.isEmpty(); }

    void appleDidReceiveToken(const QString& token);
    void appleDidReceiveRemote(const QString& title, const QString&body, const QVariantMap&data = {});

signals:
    void initializedChanged();
    void notificationReceived(const QString& title, const QString& body, const QVariantMap& data);
    void tokenChanged(const QString& token);
    // void tokenReadyChanged();
    // void tokenTimeout();

private:
    // void startWatchDog(int ms = 5000);

    bool m_initialized = false;
    // bool m_supported = false;
    // QString m_token;
    // QTimer* m_watchDog = nullptr;

#ifdef Q_OS_WIN
    QSystemTrayIcon* m_tray = nullptr;
    void ensureTray();
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

