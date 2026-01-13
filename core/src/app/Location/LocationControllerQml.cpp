// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "LocationControllerQml.h"

#include <GenesisX/Location/LocationController.h>

#include <QtQml/qqml.h>
#include <QQmlEngine>

using namespace gx::app::location;

void registerGenesisXLocation(QQmlEngine *engine)
{
    Q_UNUSED(engine);

    LocationController* locationController = new LocationController();

    qmlRegisterSingletonInstance<gx::app::location::LocationController>("GenesisX.Location", 1, 0, "LocationController", locationController);
}
