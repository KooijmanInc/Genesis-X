// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef TRANSPORTCONFIG_H
#define TRANSPORTCONFIG_H

#include <GenesisX/Orm/Core/genesisx_orm_global.h>
#include <GenesisX/Orm/Config/HttpConfig.h>
#include <GenesisX/Orm/Config/SqlConfig.h>

namespace gx::orm {

enum class Backend : quint8 {
    Http,
    Sql,
    Synchronized
};

struct GENESISX_ORM_EXPORT TransportConfig {
    Backend backend = Backend::Synchronized;
    HttpConfig http;
    SqlConfig sql;

};

}

#endif // TRANSPORTCONFIG_H
