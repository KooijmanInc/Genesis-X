// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "GXCastLifecycle.h"
#include <QGuiApplication>
#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

using namespace gx::app::cast;

GXCastLifecycle::GXCastLifecycle(QObject *parent)
    : QObject{parent}
{
    connect(qApp, &QGuiApplication::applicationStateChanged, this, &GXCastLifecycle::onAppStateChanged);
    connect(qApp, &QCoreApplication::aboutToQuit, this, [this]{
        stopJavaListener();
    });

    if (m_enabled && qApp->applicationState() == Qt::ApplicationActive)
        startJavaListener();
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
#endif
}

void GXCastLifecycle::stopJavaListener()
{
#ifdef Q_OS_ANDROID
    if (!m_started) return;
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
#endif
}

void GXCastLifecycle::onAppStateChanged(Qt::ApplicationState s)
{
    if (!m_enabled) return;
    if (s == Qt::ApplicationActive) startJavaListener();
    else stopJavaListener();
}
