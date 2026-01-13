// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef LOCATIONCONTROLLERQML_H
#define LOCATIONCONTROLLERQML_H

#include <QtQml/qqml.h>
#include <GenesisX/genesisx_global.h>

class QQmlEngine;

namespace gx::app::location {

GENESISX_CORE_EXPORT void registerGenesisXLocation(QQmlEngine* engine);

}

#endif // LOCATIONCONTROLLERQML_H
