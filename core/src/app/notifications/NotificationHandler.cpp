// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "NotificationHandler.h"
#include <QCoreApplication>

#include <src/navigation/GxRouter.h>

#ifdef Q_OS_ANDROID
#include "fcm_android.h"
#endif

#ifdef Q_OS_WIN
#include <QSystemTrayIcon>
#include <QIcon>
#include <QApplication>
#endif

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
#include <QtDBus/QtDBus>
#endif

/*!
 *  \class gx::app::notifications::NotificationHandler
 *  \inmodule GenesisX
 *  \ingroup genesisx-core
 *  \title Notification handler
 *  \since 6.10
 *  \brief Main class for local and push notifications.
 *
 *  code snippet shows the correct usage pattern:
 *  \code
 *  #include <QGuiApplication>
 *  #include <QQmlApplicationEngine>
 *  #include <GenesisX/CoreQml.h>
 *
 *  int main(int argc, char *argv[])
 *  {
 *      QGuiApplication app(argc, argv);
 *
 *      GXCore::registerEnabledQmlModules(&engine);
 *
 *      QQmlApplicationEngine engine;
 *
 *      engine.load(QStringLiteral("qrc:/views/MasterView.qml"));
 *
 *      if (engine.rootObjects().isEmpty()) return -1;
 *
 *      return app.exec();
 *  }
 *  \endcode
 *  \brief Optional you can autoload only the used modules.
 *
 *  code snippet setting modules list in qmake:
 *  \code
 *  GX_LOADED_MODULES = $$QT
 *  GX_LOADED_MODULES_CSV = $$join(GX_LOADED_MODULES, ",")
 *  DEFINES += GX_LOADED_MODULES=\\\"$$GX_LOADED_MODULES_CSV\\\"
 *  \endcode
 *  \brief And set in main.cpp.
 *  \code
 *  GXCore::registerEnabledQmlModules(&engine, GX_LOADED_MODULES);
 *  \endcode
 *
 *  \brief And implementing it in qml.
 *
 *  code snippet shows the correct usage pattern:
 *  \code
 *  import QtQuick
 *  import GenesisX.Notifications 1.0
 *
 *  Window {
 *      id: root
 *      visible: true
 *      width: 1024
 *      height: 768
 *
 *      Component.onCompleted: {
 *          notify.show("Hello from GenesisX", "Windows tray balloon :-)")
 *      }
 *
 *      NotificationHandler {
 *          id: notify
 *          onNotificationReceived: (title, body, data) => {
 *              console.log("[QML] got notification:", title, body, JSON.stringify(data))
 *          }
 *          onTokenChanged: t => console.log("New token found in qml:", t)
 *          Component.onCompleted: initialize()
 *      }
 *  }
 *  \endcode
 */

/*!
    \qmlmodule GenesisX.Notifications 1.0
    \title Genesis-X Notifications (QML)
    \brief QML APIs for local and push notifications.

    Import this module to use the \l NotificationHandler type:

    \code
    import GenesisX
    \endcode
*/

/*!
    \qmltype NotificationHandler
    \inqmlmodule GenesisX.Notifications
    \since GenesisX.Notifications 1.0
    \brief Handles local and push notifications with a unified API.

    \section2 Example
    \qml
    import GenesisX

    NotificationHandler {
        onMessageReceived: (m) => console.log(m.title, m.body)
        Component.onCompleted: requestPermission()
    }
    \endqml

    \section2 Properties
    \qmlproperty bool NotificationHandler::permissionGranted
    \qmlproperty string NotificationHandler::token
    \qmlproperty bool NotificationHandler::supported

    \section2 Signals
    \qmlsignal void NotificationHandler::tokenChanged(string token)
    \qmlsignal void NotificationHandler::messageReceived(var message)
    \qmlsignal void NotificationHandler::permissionChanged(bool granted)

    \section2 Methods
    \qmlmethod void NotificationHandler::requestPermission()
    \qmlmethod void NotificationHandler::showLocal(string text)
*/


using namespace gx::app::notifications;
using gx::navigation::router;

void onFirebaseMessage(const QVariantMap& data)
{
    QString path = data.value("route").toString();
    QVariantMap params = data;
    params.remove("route");
    qDebug() << "got the link, go to page" << path << params;
    router()->navigate(path, params);
}

void NotificationHandler::show(const QString &title, const QString &body, int msec)
{
    qDebug() << title << body << msec;
#ifdef Q_OS_WIN
    ensureTray();
    // You must have a visible tray icon for balloons to appear:
    m_tray->showMessage(title, body, qApp->windowIcon(), msec);
#endif

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    showLinuxNotification(title, body, msec);
#endif

#if defined(Q_OS_MACOS)
    showAppleNotification(title, body, msec);
#endif
}

void NotificationHandler::initialize(const QVariantMap &options)
{
    Q_UNUSED(options);
    if (m_initialized) return;

#ifdef Q_OS_ANDROID
    auto& fcm = gx::android::FcmBridge::instance();
    connect(&fcm, &gx::android::FcmBridge::messageReceived, this, &NotificationHandler::notificationReceived);
    connect(&fcm, &gx::android::FcmBridge::messageReceived, this, [](const QString& /*title*/, const QString& /*body*/, const QVariantMap& data) {
        onFirebaseMessage(data);
    });
connect(&fcm, &gx::android::FcmBridge::tokenChanged, this, &NotificationHandler::tokenChanged);
    fcm.initialize();
#elif defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    appleInitialize(options);
#else
    Q_UNUSED(options);
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

void NotificationHandler::appleDidReceiveToken(const QString &token)
{
    emit tokenChanged(token);
}

void NotificationHandler::appleDidReceiveRemote(const QString &title, const QString &body, const QVariantMap &data)
{
    emit notificationReceived(title, body, data);
}

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
void NotificationHandler::showLinuxNotification(const QString &title, const QString &body, int msec)
{
    // org.freedesktop.Notifications → Notify(app_name, replaces_id, app_icon, summary, body, actions, hints, expire_timeout)
    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning() << "[GX Notify] D-Bus session bus not available; cannot show notification.";
        return;
    }

    const QString appName = QCoreApplication::applicationName().isEmpty()
                                ? QStringLiteral("GenesisX")
                                : QCoreApplication::applicationName();

    QDBusMessage msg = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.Notifications"),
        QStringLiteral("/org/freedesktop/Notifications"),
        QStringLiteral("org.freedesktop.Notifications"),
        QStringLiteral("Notify"));

    const QString appIcon = QString();         // You can set a themed icon name or absolute file path here
    const QStringList actions;                 // e.g. {"default", "Open"} if you later wire ActionInvoked
    QVariantMap hints;                         // e.g. hints["urgency"] = (byte)1;

    msg << appName
        << static_cast<uint>(m_linuxReplacesId)
        << appIcon
        << title
        << body
        << actions
        << hints
        << static_cast<int>(msec);             // -1 uses server default

    QDBusMessage reply = QDBusConnection::sessionBus().call(msg, QDBus::Block);
    if (reply.type() == QDBusMessage::ReplyMessage && !reply.arguments().isEmpty()) {
        // Returned id lets us replace on next Notify()
        m_linuxReplacesId = reply.arguments().at(0).toUInt();
        qInfo() << "[GX Notify] D-Bus Notify success";
    } else {
        qWarning() << "[GX Notify] D-Bus Notify() failed:" << reply.errorMessage();
    }
}
#endif

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

