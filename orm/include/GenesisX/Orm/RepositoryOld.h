// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef REPOSITORYOLD_H
#define REPOSITORYOLD_H

#include <optional>
#include <functional>

#include <QList>
#include <QVariant>
#include <QVariantMap>
#include <QSqlRecord>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "GenesisX/Orm/genesisx_orm_global.h"
#include "GenesisX/Orm/DataAccess.h"

namespace gx::orm {

/**
 * Minimal contract targeted by the generator.
 */
template <typename T>
class GENESISX_ORM_EXPORT IRepository {
public:
    virtual ~IRepository() = default;

    virtual std::optional<T> find(const QVariantMap& where) = 0;
    virtual std::optional<T> findOneById(const QVariant& id) = 0;
    virtual QList<T>         findAll(const QVariantMap& where, int limit = -1, int offset = 0) = 0;
    virtual bool             save(const T& entity) = 0;
    virtual bool             saveAll(const QList<T>& entities) = 0;
};

/**
 * Optional SQL helper base.
 * Header-only, no ownership of DB. Uses your IDataAccess::ident() and qualifiedTable().
 *
 * Requirements:
 *  - Construct with (const IDataAccess&, QString flavorName, QString tableName).
 *  - Provide RowMapper that converts QSqlRecord -> T (if you use this base directly).
 */
template <typename T>
class GENESISX_ORM_EXPORT SqlRepository : public IRepository<T> {
public:
    using RowMapper = std::function<T(const QSqlRecord&)>;

    SqlRepository(const IDataAccess& da,
                  QString flavorName,
                  QString tableName,
                  RowMapper mapper)
        : m_da(da)
        , m_flavor(std::move(flavorName))
        , m_table(std::move(tableName))
        , m_mapper(std::move(mapper)) {}

protected:
    const IDataAccess& da() const { return m_da; }
    QSqlDatabase       db() const { return m_da.db(); }

    // Quote a column identifier with current flavor.
    QString q(const QString& ident) const {
        return IDataAccess::ident(m_flavor, ident);
    }

    // Fully-qualified table for this repo.
    QString qt() const {
        return m_da.qualifiedTable(m_flavor, m_da.databaseName(), m_da.schemaName(), m_table);
    }

    // Map record -> T
    T mapRow(const QSqlRecord& rec) const { return m_mapper(rec); }

    // Convenience: run prepared + params
    static bool bindAndExec(QSqlQuery& q, const QList<QPair<QString,QVariant>>& params, QString* errOut=nullptr) {
        for (const auto& p : params) q.bindValue(p.first, p.second);
        return IDataAccess::exec(q, errOut);
    }

private:
    const IDataAccess& m_da;
    QString            m_flavor;
    QString            m_table;
    RowMapper          m_mapper;
};

/**
 * Optional API helper base that wires to IDataAccess::ApiTransport.
 *
 * Requirements for T:
 *  - static T fromJson(const QJsonObject&)
 *  - QJsonObject toJson() const
 */
template <typename T>
class GENESISX_ORM_EXPORT ApiRepository : public IRepository<T> {
public:
    explicit ApiRepository(const IDataAccess& da) : m_da(da) {}

protected:
    const IDataAccess& da()  const { return m_da; }
    const ApiTransport* api() const { return m_da.api(); }

    std::optional<T> getOne(const QString& id, QString* errOut=nullptr) const {
        if (!api() || !api()->get) { setErr(errOut, "GET transport not set"); return std::nullopt; }
        const auto r = api()->get(m_da.itemRoute(id), m_da.defaultQuery(), errOut);
        if (!r) return std::nullopt;

        if (r->isObject()) {
            const auto obj = r->object();
            if (obj.contains("data") && obj.value("data").isObject())
                return T::fromJson(obj.value("data").toObject());
            return T::fromJson(obj);
        }
        setErr(errOut, "Unexpected JSON for GET item");
        return std::nullopt;
    }

    QList<T> getList(const QJsonObject& where = {}, int limit=-1, int offset=0, QString* errOut=nullptr) const {
        QList<T> out;
        if (!api() || !api()->get) { setErr(errOut, "GET transport not set"); return out; }

        QJsonObject q = m_da.defaultQuery();
        for (auto it=where.begin(); it!=where.end(); ++it) q[it.key()] = it.value();
        if (limit >= 0) q["limit"] = limit;
        if (offset > 0) q["offset"] = offset;

        const auto r = api()->get(m_da.collectionRoute(), q, errOut);
        if (!r) return out;

        if (r->isArray()) {
            for (const auto& v : r->array()) if (v.isObject()) out.push_back(T::fromJson(v.toObject()));
            return out;
        }
        if (r->isObject()) {
            const auto obj = r->object();
            auto takeArr = [&](const char* key){
                const auto a = obj.value(QLatin1String(key));
                if (!a.isArray()) return false;
                for (const auto& v : a.toArray()) if (v.isObject()) out.push_back(T::fromJson(v.toObject()));
                return true;
            };
            if (takeArr("data") || takeArr("items")) return out;
        }
        setErr(errOut, "Unexpected JSON for GET collection");
        return out;
    }

    bool postOne(const T& entity, QString* errOut=nullptr) const {
        if (!api() || !api()->post) { setErr(errOut, "POST transport not set"); return false; }
        QJsonObject body = m_da.defaultBody();
        const auto e = entity.toJson();
        for (auto it=e.begin(); it!=e.end(); ++it) body[it.key()] = it.value();
        const auto r = api()->post(m_da.collectionRoute(), body, errOut);
        return r.has_value();
    }

    bool putOne(const QString& id, const T& entity, QString* errOut=nullptr) const {
        if (!api() || !api()->put) { setErr(errOut, "PUT transport not set"); return false; }
        QJsonObject body = m_da.defaultBody();
        const auto e = entity.toJson();
        for (auto it=e.begin(); it!=e.end(); ++it) body[it.key()] = it.value();
        const auto r = api()->put(m_da.itemRoute(id), body, errOut);
        return r.has_value();
    }

    bool deleteOne(const QString& id, QString* errOut=nullptr) const {
        if (!api() || !api()->del) { setErr(errOut, "DELETE transport not set"); return false; }
        const auto r = api()->del(m_da.itemRoute(id), QJsonObject{}, errOut);
        return r.has_value();
    }

private:
    static void setErr(QString* out, const QString& v) { if (out) *out = v; }

    const IDataAccess& m_da;
};

}

#endif // REPOSITORYOLD_H
