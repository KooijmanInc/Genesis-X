// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef CONFIGIO_H
#define CONFIGIO_H

#include <GenesisX/Orm/Core/AbstractConfig.h>
#include <GenesisX/Orm/Core/genesisx_orm_global.h>
#include <GenesisX/Orm/Config/TransportConfig.h>

#include <GenesisX/Orm/Utils/Json.h>

#include <QString>

namespace gx::orm {

struct TransportConfig;

GENESISX_ORM_EXPORT bool loadTransportConfig(const AbstractConfig& config, TransportConfig& out, const QString& env = "prod", const QString& language = "");

// Json jsonHelper;

}

#endif // CONFIGIO_H
