// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef CASTQML_H
#define CASTQML_H

#include <QTimer>
#include <QDebug>
#include <QObject>
#include <QUdpSocket>
#include <QStringList>
#include <QHostAddress>
#include <QNetworkAccessManager>

#include <GenesisX/genesisx_global.h>

class QQmlEngine;
class QJSEngine;

namespace gx::app::cast {

GENESISX_CORE_EXPORT void registerGenesisXCast(QQmlEngine* engine);
}

#endif // CASTQML_H
