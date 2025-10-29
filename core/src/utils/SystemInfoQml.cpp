// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <QtQml/qqml.h>
#include <QQmlEngine>

#include "SystemInfoQml.h"
#include "include/GenesisX/utils/SystemInfo.h"

using namespace gx::utils;

QString gx::utils::SystemInfoQml::ensureAppUuid()  const { return SystemInfo::ensureAppUuid(); }
QString gx::utils::SystemInfoQml::operatingSystem() const { return SystemInfo::operatingSystem(); }
QString gx::utils::SystemInfoQml::platform()        const { return SystemInfo::platform(); }
QString gx::utils::SystemInfoQml::systemLanguage()  const { return SystemInfo::systemLanguage(); }

void registerGenesisXSystemInfo(QQmlEngine* engine)
{
    Q_UNUSED(engine);

    qmlRegisterSingletonType<gx::utils::SystemInfoQml>("GenesisX.Core.SystemInfo", 1, 0, "SystemInfo", [](QQmlEngine*, QJSEngine*) -> QObject* { return new gx::utils::SystemInfoQml; });
}
