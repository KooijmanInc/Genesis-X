// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "ConnectionControllerQml.h"
#include <QQmlEngine>

#include <GenesisX/Orm/Controllers.h>

namespace gx::orm {

static ConnectionController* s_conn = nullptr;

void registerGenesisXConnectionController(QQmlEngine* engine)
{
    Q_UNUSED(engine);

    if (!s_conn) {
        s_conn = gxOrmConnectionController();
    }

    qmlRegisterSingletonInstance("GenesisX.Orm", 1, 0, "ConnectionController", s_conn);
}

}
