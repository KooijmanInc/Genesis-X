// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef CONNECTIONFACTORY_H
#define CONNECTIONFACTORY_H

#include <memory>

#include <GenesisX/Orm/Core/genesisx_orm_global.h>
#include <GenesisX/Orm/Config/TransportConfig.h>

namespace gx::orm {

class AbstractConnection;
class HttpConnection;
class SqlConnection;

class GENESISX_ORM_EXPORT ConnectionFactory final
{
public:
    static ConnectionFactory &instance();

    ConnectionFactory(const ConnectionFactory&) = delete;

    ConnectionFactory& operator=(const ConnectionFactory&) = delete;

    void configure(const TransportConfig& config);

    Backend backend() const;

    AbstractConnection* connection() const;
    HttpConnection* http() const;
    SqlConnection* sql() const;

    void clear();

private:
    ConnectionFactory() = default;
    ~ConnectionFactory() = default;

    Backend m_backend = Backend::Http;

    std::unique_ptr<HttpConnection> m_http;
    std::unique_ptr<SqlConnection> m_sql;
};

}

#endif // CONNECTIONFACTORY_H
