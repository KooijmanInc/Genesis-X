// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Notifications/NotificationHandler.h>
#include <QCoreApplication>

#include <GenesisX/Navigation/GxRouter.h>

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
    \class gx::app::notifications::NotificationHandler
    \inmodule io.genesisx.app
    \ingroup app-classes
    \title Notification handler
    \since Qt 6.10
    \brief Main class for local and push notifications.

    code snippet shows the correct usage pattern:
   \code
   #include <QGuiApplication>
   #include <QQmlApplicationEngine>
   #include <GenesisX/CoreQml.h>

   int main(int argc, char *argv[])
   {
       QGuiApplication app(argc, argv);

       GXCore::registerEnabledQmlModules(&engine);

       QQmlApplicationEngine engine;

       engine.load(QStringLiteral("qrc:/views/MasterView.qml"));

       if (engine.rootObjects().isEmpty()) return -1;

       return app.exec();
   }
   \endcode
   \brief Optional you can autoload only the used modules.

   code snippet setting modules list in qmake:
   \code
   GX_LOADED_MODULES = $$QT
   GX_LOADED_MODULES_CSV = $$join(GX_LOADED_MODULES, ",")
   DEFINES += GX_LOADED_MODULES=\\\"$$GX_LOADED_MODULES_CSV\\\"
   \endcode
   \brief And set in main.cpp.
   \code
   GXCore::registerEnabledQmlModules(&engine, GX_LOADED_MODULES);
   \endcode

   \brief And implementing it in qml.

   code snippet shows the correct usage pattern:
   \code
   import QtQuick
   import GenesisX.Notifications 1.0

   Window {
       id: root
       visible: true
       width: 1024
       height: 768

       Component.onCompleted: {
           notify.show("Hello from GenesisX", "Windows tray balloon :-)")
       }

       NotificationHandler {
           id: notify
           onNotificationReceived: (title, body, data) => {
               console.log("[QML] got notification:", title, body, JSON.stringify(data))
           }
           onTokenChanged: t => console.log("New token found in qml:", t)
           Component.onCompleted: initialize()
       }
   }
   \endcode
 */

/*!
    \qmlmodule GenesisX.Notifications
    \inqmlmodule io.genesisx.app
    \title Genesis-X Notifications (QML)
    \since Qt 6.10
    \nativetype gx::app::notifications::NotificationHandler
    \brief QML APIs for local and push notifications.

    Import this module to use the \l NotificationHandler type:

    \code
    import GenesisX.Notifications 1.0
    \endcode
*/

/*!
    \qmltype NotificationHandler
    \inqmlmodule io.genesisx.app
    \since Qt 6.10
    \nativetype gx::app::notifications::NotificationHandler
    \brief Handles local and push notifications with a unified API.

    \section2 Example
    \qml
    import GenesisX.Notifications 1.0

    NotificationHandler {
        onMessageReceived: (m) => console.log(m.title, m.body)
        Component.onCompleted: requestPermission()
    }
    \endqml
*/

/*!
    \qmlsignal NotificationHandler::initializedChanged()

    Emitted whenever the \l initialized property changes.
*/

/*!
    \qmlsignal NotificationHandler::notificationReceived(string title,
                                                         string body,
                                                         var data)

    Emitted when a notification is received.

    The \a title and \a body contain the notification text.
    The \a data map may include extra key–value pairs from the payload.
*/

/*!
    \qmlsignal NotificationHandler::tokenChanged(string token)

    Emitted when the push-notification token changes.
    The new token is provided in \a token.
*/

using namespace gx::app::notifications;
using gx::navigation::router;

/**
 * @brief onFirebaseMessage
 * @param data
 */
void onFirebaseMessage(const QVariantMap& data)
{
    QString path = data.value("route").toString();
    QVariantMap params = data;
    params.remove("route");
    // qDebug() << "got the link, go to page" << path << params;
    router()->navigate(path, params);
}

/*!
    \qmlmethod void NotificationHandler::show(string title, string body, int msec)

    Shows notification messages in tray on desktop, show is fed by setting the \a title
    the \a body and the duration in millisecons \a msec
 */
void NotificationHandler::show(const QString &title, const QString &body, int msec)
{
    Q_UNUSED(title);
    Q_UNUSED(body);
    Q_UNUSED(msec);
#ifdef Q_OS_WIN
    ensureTray();
    // You must have a visible tray icon for balloons to appear:
    m_tray->showMessage(title, body, qApp->windowIcon(), msec);
#elif defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    showLinuxNotification(title, body, msec);
#elif defined(Q_OS_MACOS)
    showAppleNotification(title, body, msec);
#endif
}

/*!
    \qmlmethod void NotificationHandler::initialize(var options)

    Initializes the handler with the given \a options map
    The \a options argument can contain platform-specific settings
    such as initial topics or configuration flags.
 */
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
#endif

    m_initialized = true;
    emit initializedChanged();
    qInfo() << "[GX Notify] initialized";
}

void NotificationHandler::setIcon(QString _icon)
{
    icon = QIcon(_icon);
    // qDebug() << icon.
}

/*!
    \qmlmethod void NotificationHandler::subscribe(string topic)

    Allowes to subscsribe for \a topic
 */
void NotificationHandler::subscribe(const QString &topic)
{
    Q_UNUSED(topic);

#ifdef Q_OS_ANDROID
    // android::FcmBridge::instance().subscribe(topic);
#else
    qInfo() << "[GX Notify] (stub) subscribe to:" << topic;
#endif
}

/*!
    \qmlmethod NotificationHandler::unsubscribe(string topic)

    Allowes to unsubscsribe for \a topic
 */
void NotificationHandler::unsubscribe(const QString &topic)
{
    Q_UNUSED(topic);

#ifdef Q_OS_ANDROID
    // android::FcmBridge::instance().unsubscribe(topic);
#else
    qInfo() << "[GX Notify] (stub) unsubscribe from:" << topic;
#endif
}

/**
    \qmlmethod string NotificationHandler::fcmToken()
    This holds the token as string
 */
QString NotificationHandler::fcmToken() const
{
#ifdef Q_OS_ANDROID
    return gx::android::FcmBridge::instance().token();
#else
    return {};
#endif
}

/*!
    \qmlproperty bool NotificationHandler::initialized

    True if the user has initialized notifications
 */
bool NotificationHandler::initialized() const
{
    return m_initialized;
}

/*!
    Sends signal if token is received for Apple devices
    \a token The APNs device token.
 */
void NotificationHandler::appleDidReceiveToken(const QString &token)
{
    emit tokenChanged(token);
}

/*!
    Sends signal if a notification has been received
    \a title, The title of the notification
    \a body, The body of the notification
    \a data, variant map of the notifiation
 */
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

    if (icon.isNull()) {
        QIcon icon = qApp->windowIcon();
    }
    if (icon.isNull()) {
        icon = QIcon(":/logo.ico");
    }
    m_tray->setIcon(icon);

    m_tray->setToolTip(qApp->applicationDisplayName().isEmpty()
                           ? QStringLiteral("GenesisX")
                           : qApp->applicationDisplayName());
    m_tray->setToolTip("Genesis-X");
    m_tray->show();
    qApp->installEventFilter(this);
}

bool NotificationHandler::eventFilter(QObject*, QEvent* e)
{
    if (e->type() == QEvent::ApplicationWindowIconChange && m_tray) {
        if (icon.isNull()) {
            QIcon icon = qApp->windowIcon();
        }
        if (!icon.isNull()) m_tray->setIcon(icon);
    }
    return false;
}
#endif

