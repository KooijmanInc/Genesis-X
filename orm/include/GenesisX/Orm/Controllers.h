// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef CONTROLLERS_H
#define CONTROLLERS_H

#include <GenesisX/Orm/genesisx_orm_global.h>

#include <GenesisX/Orm/ConnectionController.h>
#include <GenesisX/Orm/CommandController.h>

class QObject;

namespace gx::orm {

class ConnectionController;
class CommandController;

}

namespace gx::orm {

GENESISX_ORM_EXPORT ConnectionController* gxOrmConnectionController();
GENESISX_ORM_EXPORT CommandController* gxOrmCommandController();

GENESISX_ORM_EXPORT void gxOrmSetConnectionController(ConnectionController* conn);

}

#endif // CONTROLLERS_H
