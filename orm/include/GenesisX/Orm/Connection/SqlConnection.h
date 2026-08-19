// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef SQLCONNECTION_H
#define SQLCONNECTION_H

#include <QSqlDatabase>

#include <GenesisX/Orm/Connection/AbstractConnection.h>
#include <GenesisX/Orm/Connection/SqlSchema.h>

namespace th::environment {
    class AbstractEnvironmentResourceRepository;
}

namespace gx::orm {

enum class DatabaseEngine
{
    MySql,
    MariaDB,
    PostgreSQL,
    MicrosoftSQLServer,
    SQLite
};

class GENESISX_ORM_EXPORT SqlConnection final
    : public AbstractConnection
{
    Q_OBJECT

public:
    explicit SqlConnection(
        const SqlConfig &config,
        QObject *parent = nullptr
        );
    ~SqlConnection() override;

    QFuture<ConnectionResult> ping() override;

    QFuture<ConnectionResult> insert(const QString& table, const QVariantMap& bindings);
    QFuture<ConnectionResult> select(const QString& table, const QVariantMap& bindings = {}, QString conditions = {});

    QFuture<ConnectionResult> execute(const QString& statement, const QVariantMap& bindings = {}, const QString& count = {});

    QSqlDatabase database() const;

    QString selectColumns = "*";

private:
    void configureDatabase();

    SqlConfig m_config;
    QString m_connectionName;
    QSqlDatabase m_database;

    SqlSchema* schema();
    SqlSchema* m_schema = nullptr;

    friend class ::th::environment::AbstractEnvironmentResourceRepository;
};

}

#endif // SQLCONNECTION_H
