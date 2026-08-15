// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/Connection/ConnectionFactory.h>

#include <GenesisX/Orm/Connection/AbstractConnection.h>
#include <GenesisX/Orm/Connection/HttpConnection.h>
#include <GenesisX/Orm/Connection/SqlConnection.h>

using namespace gx::orm;

ConnectionFactory &ConnectionFactory::instance()
{
    static ConnectionFactory factory;

    return factory;
}

void gx::orm::ConnectionFactory::configure(const TransportConfig &config)
{
    m_backend = config.backend;

    m_http.reset();
    m_sql.reset();

    switch (m_backend) {
    case Backend::Http:
        m_http = std::make_unique<HttpConnection>(config.http);
        break;
    case Backend::Sql:
        m_sql = std::make_unique<SqlConnection>(config.sql);
        break;
    case Backend::Synchronized:
        m_http = std::make_unique<HttpConnection>(config.http);
        m_sql = std::make_unique<SqlConnection>(config.sql);
        break;
    }
}

Backend gx::orm::ConnectionFactory::backend() const
{
    return m_backend;
}

gx::orm::AbstractConnection *gx::orm::ConnectionFactory::connection() const
{
    switch (m_backend) {
    case Backend::Http:
        return m_http.get();

    case Backend::Sql:
    case Backend::Synchronized:
        return m_sql.get();
    }

    return nullptr;
}

HttpConnection *ConnectionFactory::http() const
{
    return m_http.get();
}

SqlConnection *ConnectionFactory::sql() const
{
    return m_sql.get();
}
