// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <QtQml/qqml.h>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QStyleHints>
#include <QQmlEngine>

#ifdef Q_OS_WIN
#include <QSettings>
#include <qt_windows.h>
#endif

#include "SystemInfoQml.h"
#include "include/GenesisX/utils/SystemInfo.h"

using namespace gx::utils;

SystemInfoQml::SystemInfoQml(QObject *parent)
    : QObject{parent}
{
    QStyleHints *styleHints = QGuiApplication::styleHints();

    styleHints->unsetColorScheme();

    // m_colorScheme = styleHints->colorScheme();

    connect(
        styleHints,
        &QStyleHints::colorSchemeChanged,
        this,
        [this](Qt::ColorScheme colorScheme)
        {
            if (m_colorScheme == colorScheme) {
                return;
            }

            m_colorScheme = colorScheme;
            emit colorSchemeChanged();
        }
    );

#ifdef Q_OS_WIN
    QCoreApplication::instance()->installNativeEventFilter(this);
#endif

    updateColorScheme();
}

SystemInfoQml::~SystemInfoQml()
{
#ifdef Q_OS_WIN
    if (QCoreApplication::instance()) {
        QCoreApplication::instance()->removeNativeEventFilter(this);
    }
#endif
}

Qt::ColorScheme SystemInfoQml::colorScheme() const
{
    return m_colorScheme;
}

bool SystemInfoQml::darkMode() const
{
    return m_colorScheme == Qt::ColorScheme::Dark;
}

bool SystemInfoQml::nativeEventFilter(
    const QByteArray &eventType,
    void *message,
    qintptr *result
    )
{
    Q_UNUSED(eventType)
    Q_UNUSED(result)

#ifdef Q_OS_WIN
    const MSG *msg = static_cast<MSG *>(message);

    if (msg->message == WM_SETTINGCHANGE) {
        // Delay reading slightly because Windows can broadcast the event
        // before all personalization values are available.
        QMetaObject::invokeMethod(
            this,
            &SystemInfoQml::updateColorScheme,
            Qt::QueuedConnection
            );
    }
#else
    Q_UNUSED(message)
#endif

    return false;
}

Qt::ColorScheme SystemInfoQml::systemColorScheme() const
{
#ifdef Q_OS_WIN
    QSettings settings(
        QStringLiteral(
            "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\"
            "CurrentVersion\\Themes\\Personalize"
            ),
        QSettings::NativeFormat
        );

    const int appsUseLightTheme =
        settings.value(QStringLiteral("AppsUseLightTheme"), 1).toInt();

    return appsUseLightTheme == 0
               ? Qt::ColorScheme::Dark
               : Qt::ColorScheme::Light;
#else
    return QGuiApplication::styleHints()->colorScheme();
#endif
}

void SystemInfoQml::updateColorScheme()
{
    const Qt::ColorScheme newScheme = systemColorScheme();

    if (m_colorScheme == newScheme) {
        return;
    }

    m_colorScheme = newScheme;
    emit colorSchemeChanged();
}

QString gx::utils::SystemInfoQml::ensureAppUuid()  const { return SystemInfo::ensureAppUuid(); }
QString gx::utils::SystemInfoQml::operatingSystem() const { return SystemInfo::operatingSystem(); }
QString gx::utils::SystemInfoQml::platform()        const { return SystemInfo::platform(); }
QString gx::utils::SystemInfoQml::systemLanguage()  const { return SystemInfo::systemLanguage(); }

void registerGenesisXSystemInfo(QQmlEngine* engine)
{
    Q_UNUSED(engine);

    qmlRegisterSingletonType<gx::utils::SystemInfoQml>("GenesisX.SystemInfo", 1, 0, "SystemInfo", [](QQmlEngine*, QJSEngine*) -> QObject* { return new gx::utils::SystemInfoQml; });
}
