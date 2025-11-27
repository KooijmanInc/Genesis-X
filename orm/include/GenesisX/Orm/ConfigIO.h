// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef CONFIGIO_H
#define CONFIGIO_H

#include <GenesisX/Orm/genesisx_orm_global.h>
#include <GenesisX/Orm/TransportConfig.h>
#include <QString>

namespace gx::orm {

struct TransportConfig;

GENESISX_ORM_EXPORT bool loadTransportConfig(const QString& path, TransportConfig& out, const QStringView& env = {});

}

#endif // CONFIGIO_H
