// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "PermissionsQml.h"

#include <QtQml/qqml.h>
#include <QQmlEngine>

#include "include/GenesisX/Permissions/Permissions.h"
#include "PermissionsQml.h"

using namespace gx::app::permissions;

void registerGenesisXPermissions(QQmlEngine* engine)
{
    Q_UNUSED(engine);

    qmlRegisterType<gx::app::permissions::Permissions>("GenesisX.App.Permissions", 1, 0, "Permissions");
}
