// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef ORMQML_H
#define ORMQML_H

#include <QString>

#include "genesisx_orm_global.h"

class QQmlEngine;

namespace gx::orm {

GENESISX_ORM_EXPORT void registerEnabledQmlModules(QQmlEngine* engine);

}

#endif // ORMQML_H
