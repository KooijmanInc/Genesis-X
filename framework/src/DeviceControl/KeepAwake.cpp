// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Framework/DeviceControl/KeepAwake.h>

#include <QCoreApplication>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

using namespace gx::framework;

KeepAwake::KeepAwake(QObject *parent)
    : QObject{parent}
{
}

void KeepAwake::setEnabled(bool on)
{
    if (m_enabled == on) return;

    m_enabled = on;
    applyPlatform(on);

    emit enabledChanged();
}

void KeepAwake::applyPlatform(bool on)
{
#ifdef Q_OS_ANDROID
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([on]() -> QVariant {
        QJniObject activity = QNativeInterface::QAndroidApplication::context();

        if (!activity.isValid()) return {};

        QJniObject window = activity.callObjectMethod(
            "getWindow", "()Landroid/view/Window;"
        );

        if (!window.isValid()) return {};

        const jint FLAG_KEEP_SCREEN_ON = 128;

        if (on) {
            window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
        } else {
            window.callMethod<void>("clearFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
        }

        return {};
    });
#endif
}
