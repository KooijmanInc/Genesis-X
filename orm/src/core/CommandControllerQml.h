// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef COMMANDCONTROLLERQML_H
#define COMMANDCONTROLLERQML_H

#include "include/GenesisX/Orm/genesisx_orm_global.h"

class QQmlEngine;

namespace gx::orm {

GENESISX_ORM_EXPORT void registerGenesisXCommandController(QQmlEngine* engine);

}

#endif // COMMANDCONTROLLERQML_H
