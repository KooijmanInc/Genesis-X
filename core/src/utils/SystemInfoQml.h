// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef SYSTEMINFOQML_H
#define SYSTEMINFOQML_H

#include <QObject>
#include <QtQml/qqml.h>

#include "include/GenesisX/genesisx_global.h"

class QQmlEngine;

namespace gx::utils {

GENESISX_CORE_EXPORT void registerGenesisXSystemInfo(QQmlEngine* engine);

class GENESISX_CORE_EXPORT SystemInfoQml : public QObject
{
    Q_OBJECT
    QML_SINGLETON

public:
    explicit SystemInfoQml(QObject* parent = nullptr) : QObject{parent} {}

    Q_INVOKABLE QString ensureAppUuid() const;
    Q_INVOKABLE QString operatingSystem() const;
    Q_INVOKABLE QString platform() const;
    Q_INVOKABLE QString systemLanguage() const;
};

}

#endif // SYSTEMINFOQML_H
