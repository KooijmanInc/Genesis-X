// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/Connection/SqlConnection.h>

#include <QtConcurrent>
#include <QElapsedTimer>
#include <QSqlRecord>
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
        .message = ""
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

QFuture<ConnectionResult> SqlConnection::insert(const QString& table, const QVariantMap &bindings)
{
    QStringList columns;
    QVariantMap bind;
    QStringList binds;

    for (auto i = bindings.cbegin(), end = bindings.cend(); i != end; ++i) {
        columns.append(i.key());
        bind[":" + i.key()] = i.value();
        binds.append(":" + i.key());
    }

    QString query = "INSERT INTO ";
    query += table;
    query += " (";
    query += columns.join(", ");
    query += ") VALUES (";
    query += binds.join(", ");
    query += ")";

    return execute(query, bind);
}

QFuture<ConnectionResult> SqlConnection::select(const QString &table, const QVariantMap &bindings, QString conditions)
{
    Q_UNUSED(conditions)
    QVariantMap bind;
    QStringList binds;
    QString where;

    for (auto i = bindings.cbegin(), end = bindings.cend(); i != end; ++i) {
        bind[":" + i.key()] = i.value();
        binds.append(":" + i.key());
        where += " " + i.key() + " = :" + i.key() + " AND";
    }

    QString query = "SELECT ";
    query += selectColumns;
    query += " FROM ";
    query += table;
    if (!where.isEmpty()) {
        query += " WHERE";
        query += where;
        query.chop(4);
    }

    return execute(query, bind);
}

QFuture<ConnectionResult> SqlConnection::execute(const QString& statement, const QVariantMap& bindings, const QString& count)
{
    if (!m_database.isOpen()) {
        if (!m_database.open()) {
            return QtFuture::makeReadyValueFuture(
                ConnectionResult{
                    .successful = false,
                    .statusCode = 0,
                    .message = m_database.lastError().text()
                }
            );
        }
    }

    QSqlQuery query{m_database};

    if (!query.prepare(statement)) {
        return QtFuture::makeReadyValueFuture(
            ConnectionResult{
                .successful = false,
                .statusCode = 0,
                .message = query.lastError().text()
            }
        );
    }

    for (auto it = bindings.cbegin(); it != bindings.cend(); ++it) {
        query.bindValue(it.key(), it.value());
    }

    if (!query.exec()) {
        return QtFuture::makeReadyValueFuture(
            ConnectionResult{
                .successful = false,
                .statusCode = 0,
                .message = query.lastError().text()
            }
        );
    }

    QueryResult sqlResult;

    const QSqlRecord record = query.record();

    for (int i = 0; i < record.count(); ++i) {
        sqlResult.columns.append(record.fieldName(i));
    }

    while (query.next()) {
        QVariantMap row;

        if (!count.isEmpty()) {
            row.insert(count, query.value(0).toInt());
        } else {
            for (int i = 0; i < record.count(); ++i) {
                row.insert(
                    record.fieldName(i),
                    query.value(i)
                );
            }
        }

        sqlResult.rows.push_back(std::move(row));
    }
    // qDebug() << "bindings: " << bindings;
    // qDebug() << "count: " << count;
    // qDebug() << "statement: " << statement;
    // // if (!count.isEmpty()) {
    //     // qDebug() << "\narrived\n" << query.value("rows");
    //     while (query.next()) {
    //         for (int i = 0; i < record.count(); ++i) {
    //             qDebug() << "value is: " << query.value(i);
    //         }
    //     }
    // }

    // if (select) {
    //     return QtFuture::makeReadyValueFuture(
    //         ConnectionResult{
    //             .successful = true,
    //             .statusCode = 0,
    //             .message = query.result()
    //         }
    //         );
    // } else {
        return QtFuture::makeReadyValueFuture(
            ConnectionResult{
                .successful = true,
                .statusCode = 0,
                .message = "SQL execution successful: " + query.lastInsertId().toString(),
            .result = std::move(sqlResult)
            }
        );
    // }

    // auto success = ping().then([](const ConnectionResult& result) {
    //     return result;
    // });
    // qDebug() << "in execute " << success.result().message;
    // return QtConcurrent::run(
    //     [this, statement, bindings]() -> ConnectionResult
    //     {
    //         QSqlQuery query{m_database};

    //         if (!query.prepare(statement)) {
    //             return {
    //                 .successful = false,
    //                 .statusCode = 0,
    //                 .message = query.lastError().text()
    //             };
    //         }



    //         if (!query.exec()) {
    //             return {
    //                 .successful = false,
    //                 .statusCode = 0,
    //                 .message = query.lastError().text()
    //             };
    //         }

    //         return {
    //             .successful = true,
    //             .statusCode = 0,
    //             .message = "SQL execution successful"
    //         };
    //     }
    //     );
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

    if (!m_config.pass.isEmpty()) {
        m_database.setPassword(
            m_config.pass
            );
    }
}

SqlSchema* SqlConnection::schema()
{
    auto success = ping().then([](const ConnectionResult& result) {
        return result;
    });
    if (success.result().successful) {
        m_schema = new SqlSchema();
        // m_schema->setSqlConnection(m_database);
    }

    return m_schema;
}