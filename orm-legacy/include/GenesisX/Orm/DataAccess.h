// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef DATAACCESS_H
#define DATAACCESS_H

#include <QJsonDocument>
#include <QSqlDatabase>
#include <QJsonObject>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>

#include <GenesisX/Orm/genesisx_orm_global.h>

namespace gx::orm {

struct GENESISX_ORM_EXPORT ApiTransport
{
    std::function<std::optional<QJsonDocument>(const QString& route, const QJsonObject& queryOrBody, QString* errOut)> get;
    std::function<std::optional<QJsonDocument>(const QString& route, const QJsonObject&  body, QString* errOut)> post;
    std::function<std::optional<QJsonDocument>(const QString& route, const QJsonObject&  body, QString* errOut)> put;
    std::function<std::optional<QJsonDocument>(const QString& route, const QJsonObject&  body, QString* errOut)> del;
};

/**
 * @brief The IDataAccess class
 *
 * Very small, stable contract that generated *DataAccess classes* will implement.
 * It does NOT own a connection; it only names it. Your app is free to create and
 * manage the QSqlDatabase connections however it wants (ConnectionController, etc.).
 */
class GENESISX_ORM_EXPORT IDataAccess
{
public:
    enum class Backend { Sql, Api };

    virtual ~IDataAccess() = default;

    // ---- Common contract (implemented by generated *DataAccess) ----
    virtual Backend backend() const = 0;
    virtual QString tableName() const = 0;

    // ---- SQL side(used when backend() == Sql) ----
    virtual QString connectionName() const { return {}; }
    virtual QString databaseName() const { return {}; }
    virtual QString schemaName() const { return {}; }

    // --- API side (used when backend()==Api) -------------------------------
    virtual QString baseRoute()    const { return {}; }  // e.g. "/api/v1"
    virtual QString resourceName() const { return {}; }  // e.g. "push-notification-tokens"

    // Optional default headers/payload fragments. Leave empty if unused.
    virtual QJsonObject defaultQuery() const { return {}; }
    virtual QJsonObject defaultBody() const { return {}; }

    // ---------- Convenience (header-only) ----------
    QSqlDatabase db() const { return QSqlDatabase::database(connectionName()); }

    // App injects transport at runtime (not owned).
    void setApiTransport(const ApiTransport* t) { m_api = t; }
    const ApiTransport* api() const { return m_api; }

    // Helpers to build routes
    QString collectionRoute() const {
        if (baseRoute().isEmpty()) return resourceName();
        return baseRoute().endsWith('/') ? baseRoute() + resourceName()
                                         : baseRoute() + "/" + resourceName();
    }

    QString itemRoute(const QString& id) const {
        const QString col = collectionRoute();
        return col.isEmpty() ? id : (col + "/" + id);
    }

    // vendor-quoting helpers + qualified table
    static QString ident(const QString& flavor, const QString& name) {
        if (name.isEmpty()) return name;
        if (flavor == "MySql")      return QString("`%1`").arg(QString(name).replace('`',"``"));
        if (flavor == "PostgreSQL") return QString("\"%1\"").arg(QString(name).replace('"',"\"\""));
        if (flavor == "SQLite")     return QString("\"%1\"").arg(QString(name).replace('"',"\"\""));
        if (flavor == "MSSQL")      return QString("[%1]").arg(QString(name).replace(']',"]]"));
        if (flavor == "Oracle")     return QString("\"%1\"").arg(QString(name).replace('"',"\"\""));
        return name;
    }

    // Build a qualified table identifier for SELECTs (dbms-neutral-ish)
    QString qualifiedTable(const QString& flavor, const QString& db, const QString& schema, const QString& table) const {
        const QString t = ident(flavor, table);
        if (flavor == "MySql")      return db.isEmpty()     ? t : ident(flavor, db) + "." + t;
        if (flavor == "PostgreSQL") return (schema.isEmpty()? ident(flavor, db) : ident(flavor, schema)) + "." + t;
        if (flavor == "MSSQL")      return (schema.isEmpty()? QStringLiteral("[dbo]") : ident(flavor, schema)) + "." + t;
        if (flavor == "SQLite")     return t;
        if (flavor == "Oracle")     return (schema.isEmpty()? ident(flavor, db) : ident(flavor, schema)) + "." + t;
        return t;
    }

    // Tiny exec helper
    static bool exec(QSqlQuery& q, QString* err = nullptr) {
        const bool ok = q.exec();
#ifndef NDEBUG
        if (!ok) {
            qDebug() << "gx::orm::IDataAccess::exec error:" << q.lastError().text()
            << "SQL:" << q.lastQuery();
        }
#endif
        if (!ok && err) *err = q.lastError().text();
        return ok;
    }

private:
    const ApiTransport* m_api = nullptr;
};

}

#endif // DATAACCESS_H
