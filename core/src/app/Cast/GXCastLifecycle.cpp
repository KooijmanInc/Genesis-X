// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "GXCastLifecycle.h"
#include <QCoreApplication>
#include <QGuiApplication>
#include <QPointer>
#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

using namespace gx::app::cast;

GXCastLifecycle::GXCastLifecycle(QObject *parent)
    : QObject{parent}
{
#ifdef Q_OS_ANDROID
    auto setupConnections = [this]() {
        auto *core = QCoreApplication::instance();
        if (!core)
            return;

        // aboutToQuit is on QCoreApplication
        connect(core, &QCoreApplication::aboutToQuit, this, [this]{
            stopJavaListener();
        });

        // applicationStateChanged is on QGuiApplication
        auto *gui = qobject_cast<QGuiApplication*>(core);
        if (!gui)
            return;

        connect(gui, &QGuiApplication::applicationStateChanged,
                this, &GXCastLifecycle::onAppStateChanged);

        if (m_enabled && gui->applicationState() == Qt::ApplicationActive)
            startJavaListener();
    };

    if (!QCoreApplication::instance()) {
        // Constructed too early (before app exists): defer to next event loop turn
        QPointer<GXCastLifecycle> self(this);
        QMetaObject::invokeMethod(this, [self, setupConnections] {
            if (!self) return;
            setupConnections();
        }, Qt::QueuedConnection);
        return;
    }

    setupConnections();
    // auto* app = QGuiApplication::instance();
    // if (!app) {
    //     QMetaObject::invokeMethod(this, [this]{
    //         auto *app2 = QGuiApplication::instance();
    //         if (!app2) return;

    //         connect(app2, &QGuiApplication::applicationStateChanged,
    //                 this, &GXCastLifecycle::onAppStateChanged);

    //         connect(app2, &QCoreApplication::aboutToQuit,
    //                 this, [this]{ stopJavaListener(); });

    //         if (m_enabled && app2->applicationState() == Qt::ApplicationActive)
    //             startJavaListener();
    //     }, Qt::QueuedConnection);

    //     return;
    // }
    // connect(app, &QGuiApplication::applicationStateChanged, this, &GXCastLifecycle::onAppStateChanged);
    // connect(app, &QCoreApplication::aboutToQuit, this, [this]{
    //     stopJavaListener();
    // });

    // if (m_enabled && app->applicationState() == Qt::ApplicationActive)
    //     startJavaListener();
#endif
}

void GXCastLifecycle::disconnect()
{
    m_enabled = false;
    m_started = false;
#ifdef Q_OS_ANDROID
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=] {
    QJniObject ctx = QNativeInterface::QAndroidApplication::context();
    if (ctx.isValid()) {
        QJniObject::callStaticMethod<void>(
            "com/genesisx/cast/GXCastManager",
            "disconnect",
            "(Landroid/content/Context;)V",
            ctx
            );
    }
    });
#endif
}

void GXCastLifecycle::setEnabled(bool on)
{
    if (m_enabled == on) return;
    m_enabled = on;
    emit enabledChanged();
    if (!m_enabled) stopJavaListener();
    else if (qApp->applicationState() == Qt::ApplicationActive) startJavaListener();
}

void GXCastLifecycle::startJavaListener()
{
#ifdef Q_OS_ANDROID
    if (m_started) return;

    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([this]() -> QVariant {
        QJniObject activity = QNativeInterface::QAndroidApplication::context();
        if (activity.isValid()) {
            QJniObject::callStaticMethod<void>(
                "com/genesisx/cast/GXCastManager",
                "onStart",
                "(Landroid/app/Activity;)V",
                activity.object<jobject>()
                );
            m_started = true;
        }
        return {};
    });
#endif
}

void GXCastLifecycle::stopJavaListener()
{
#ifdef Q_OS_ANDROID
    if (!m_started) return;

    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([this]() -> QVariant {
        QJniObject activity = QNativeInterface::QAndroidApplication::context();
        if (activity.isValid()) {
            QJniObject::callStaticMethod<void>(
                "com/genesisx/cast/GXCastManager",
                "onStop",
                "(Landroid/app/Activity;)V",
                activity.object<jobject>()
                );
            m_started = false;
        }
        return {};
    });
#endif
}

void GXCastLifecycle::onAppStateChanged(Qt::ApplicationState s)
{
    if (!m_enabled) return;
    if (s == Qt::ApplicationActive) startJavaListener();
    else stopJavaListener();
}
