// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/ConnectionController.h>
#include <GenesisX/Orm/CommandController.h>
#include <GenesisX/Orm/Controllers.h>
#include <QCoreApplication>

static gx::orm::ConnectionController* s_conn = nullptr;
static gx::orm::CommandController* s_ctrl = nullptr;

namespace gx::orm {

ConnectionController *gxOrmConnectionController()
{
    if (!s_conn) {
        s_conn = new ConnectionController(qApp);
    }

    return s_conn;
}

CommandController *gxOrmCommandController()
{
    if (!s_ctrl) {
        s_ctrl = new CommandController(gxOrmConnectionController(), qApp);
    }
    return s_ctrl;
}

void gxOrmSetConnectionController(ConnectionController *conn)
{
    if (s_ctrl) return;

    s_conn = conn;
}

}
