// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/Connection/SqlConnection.h>

#include <QElapsedTimer>
#include <QSqlError>
#include <QSqlQuery>
#include <QFuture>
#include <QUuid>

using namespace gx::orm;

SqlConnection::SqlConnection(
    const SqlConfig &config,
    QObject *parent
    )
    : AbstractConnection{parent}
    , m_config{config}
    , m_connectionName{
    QStringLiteral("genesisx_%1").arg(
              QUuid::createUuid().toString(
              QUuid::WithoutBraces
              )
        )
    }
{
    configureDatabase();
}

SqlConnection::~SqlConnection()
{
    if (m_database.isValid()) {
        m_database.close();
    }

    /*
     * All QSqlDatabase handles must be released before calling
     * QSqlDatabase::removeDatabase().
     */
    m_database = QSqlDatabase{};

    QSqlDatabase::removeDatabase(
        m_connectionName
        );
}

QFuture<ConnectionResult> SqlConnection::ping()
{
    QElapsedTimer timer;
    timer.start();

    ConnectionResult result{
        .backend = Backend::Sql,
    };

    if (!m_database.isValid()) {
        result.latencyMs =
            static_cast<int>(timer.elapsed());

        result.message = m_config.driver.isEmpty()
                             ? QStringLiteral(
                                   "No SQL driver configured"
                                   )
                             : QStringLiteral(
                                   "SQL connection is invalid"
                                   );

        return QtFuture::makeReadyValueFuture(
            result
            );
    }

    if (
        !m_database.isOpen()
        && !m_database.open()
        ) {
        result.latencyMs =
            static_cast<int>(timer.elapsed());

        result.message =
            m_database.lastError().text();

        return QtFuture::makeReadyValueFuture(
            result
            );
    }

    QSqlQuery query{m_database};

    result.successful =
        query.exec(QStringLiteral("SELECT 1"));

    result.latencyMs =
        static_cast<int>(timer.elapsed());

    result.message = result.successful
                         ? QStringLiteral(
                               "SQL connection successful"
                               )
                         : query.lastError().text();

    return QtFuture::makeReadyValueFuture(
        result
        );
}

QSqlDatabase SqlConnection::database() const
{
    return m_database;
}

void SqlConnection::configureDatabase()
{
    if (m_config.driver.isEmpty()) {
        return;
    }

    m_database = QSqlDatabase::addDatabase(
        m_config.driver,
        m_connectionName
        );

    m_database.setDatabaseName(
        m_config.database
        );

    if (!m_config.host.isEmpty()) {
        m_database.setHostName(
            m_config.host
            );
    }

    if (m_config.port > 0) {
        m_database.setPort(
            m_config.port
            );
    }

    if (!m_config.user.isEmpty()) {
        m_database.setUserName(
            m_config.user
            );
    }

    if (!m_config.password.isEmpty()) {
        m_database.setPassword(
            m_config.password
            );
    }
}