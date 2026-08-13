// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef TRANSPORTCONFIG_H
#define TRANSPORTCONFIG_H

#include <GenesisX/Orm/genesisx_orm_global.h>
#include <GenesisX/Orm/HttpConfig.h>
#include <GenesisX/Orm/SqlConfig.h>

namespace gx::orm {

enum class Backend : quint8 { Http, Sql };

struct GENESISX_ORM_EXPORT TransportConfig {
    Backend backend = Backend::Http;
    HttpConfig http;
    SqlConfig sql;
};

}

#endif // TRANSPORTCONFIG_H
