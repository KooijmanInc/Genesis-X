// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

#include <GenesisX/Orm/ConfigIO.h>
#include <GenesisX/Orm/TransportConfig.h>

namespace gx::orm {

static QJsonObject loadObj(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return {};
    auto d = QJsonDocument::fromJson(f.readAll());
    return d.isObject() ? d.object() : QJsonObject{};
}

static void mergeHttp(HttpConfig& http, const QJsonObject& api) {
    if (api.contains("baseUrl"))         http.baseUrl = QUrl(api.value("baseUrl").toString());
    if (api.contains("appVersion"))      http.appVersion = api.value("appVersion").toString();
    if (api.contains("userLanguage"))    http.userLanguage = api.value("userLanguage").toString(http.userLanguage);
    if (api.contains("timeoutMs"))       http.timeoutMs = api.value("timeoutMs").toInt(http.timeoutMs);
    if (api.contains("retryCount"))      http.retryCount = api.value("retryCount").toInt(http.retryCount);
    if (api.contains("allowInsecureDev"))http.allowInsecureDev = api.value("allowInsecureDev").toBool(http.allowInsecureDev);
    if (api.contains("appKey"))          http.appKey = api.value("appKey").toString(http.appKey);
    if (api.contains("apiToken"))        http.apiToken = api.value("apiToken").toString(http.apiToken);
    if (api.contains("bearerToken"))     http.bearerToken = api.value("bearerToken").toString(http.bearerToken);

    if (api.contains("headers")) {
        const auto headers = api.value("headers").toObject();
        for (auto it = headers.begin(); it != headers.end(); ++it)
            http.defaultHeaders.insert(it.key().toUtf8(), it.value().toString().toUtf8());
    }
}

static void mergeSql(SqlConfig& sql, const QJsonObject& s) {
    if (s.contains("driver"))   sql.driver   = s.value("driver").toString(sql.driver);
    if (s.contains("host"))     sql.host     = s.value("host").toString(sql.host);
    if (s.contains("port"))     sql.port     = s.value("port").toInt(sql.port);
    if (s.contains("database")) sql.database = s.value("database").toString(sql.database);
    if (s.contains("user"))     sql.user     = s.value("user").toString(sql.user);
    if (s.contains("password")) sql.password = s.value("password").toString(sql.password);

    if (s.contains("options")) {
        const auto opts = s.value("options").toObject();
        for (auto it = opts.begin(); it != opts.end(); ++it)
            sql.options.insert(it.key(), it.value().toVariant());
    }
}

bool loadTransportConfig(const QString &path, TransportConfig& out, const QStringView &env)
{
    const auto root = loadObj(path);
    if (root.isEmpty()) return false;

    const auto b = root.value("backend").toString().toLower();
    out.backend = (b == "sql") ? Backend::Sql : Backend::Http;

    const auto api = root.value("api").toObject();
    const auto sql = root.value("sql").toObject();
    if (!api.isEmpty()) mergeHttp(out.http, api);
    if (!sql.isEmpty()) mergeSql(out.sql, sql);

    if (!env.isEmpty()) {
        const auto overrides = root.value("overrides").toObject();
        const auto ov = overrides.value(QString(env)).toObject();
        if (!ov.isEmpty()) {
            const auto apiOv = ov.value("api").toObject();
            const auto sqlOv = ov.value("sql").toObject();
            if (!apiOv.isEmpty()) mergeHttp(out.http, apiOv);
            if (!sqlOv.isEmpty()) mergeSql(out.sql, sqlOv);
        }
    }

    // qInfo() << "[CFG] backend=" << (cfg.back);

    return true;
}

}
