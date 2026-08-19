// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef CONNECTIONRESULT_H
#define CONNECTIONRESULT_H

#include <QString>

#include <GenesisX/Orm/Core/genesisx_orm_global.h>
#include <GenesisX/Orm/Config/TransportConfig.h>

namespace gx::orm {

struct QueryResult
{
    QStringList columns;
    std::vector<QVariantMap> rows;
};

struct GENESISX_ORM_EXPORT ConnectionResult
{
    Backend backend = Backend::Http;
    bool successful = false;
    int latencyMs = -1;
    int statusCode = 0;
    QString message;

    QueryResult result = {};
};

}

#endif // CONNECTIONRESULT_H
