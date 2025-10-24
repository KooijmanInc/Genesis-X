// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "include/GenesisX/Orm/ApiClient.h"

#include <QJsonDocument>

static QByteArray toJsonBytes(const QJsonObject& o) {
    return QJsonDocument(o).toJson(QJsonDocument::Compact);
}

gx::orm::ApiClient::ApiClient(const QUrl &baseUrl, QString appKey, QString apiToken, QString appVersion, QObject *parent)
    : QObject{parent}
    , m_baseUrl(baseUrl)
    , m_appKey(std::move(appKey))
    , m_apiToken(std::move(apiToken))
    , m_appVersion(std::move(appVersion))
{

}

void gx::orm::ApiClient::setToken(QString t)
{
    m_jwt = std::move(t);
    if (m_persist && !m_org.isEmpty() && !m_app.isEmpty()) {
        QSettings s(m_org, m_app);
        s.setValue(m_storeKey, m_jwt);
        s.sync();
    }
}

void gx::orm::ApiClient::setTokenPersistance(bool enabled, QString org, QString app, QString key)
{
    m_persist = enabled;
    m_org = std::move(org);
    m_app = std::move(app);
    m_storeKey = std::move(key);

    if (m_persist && !m_org.isEmpty() && !m_app.isEmpty()) {
        QSettings s(m_org, m_app);
        const auto stored = s.value(m_storeKey).toString();
        if (!stored.isEmpty())
            m_jwt = stored;
    }
}

void gx::orm::ApiClient::login(const QString &username, const QString &password)
{
    const QString path = QStringLiteral("/login_check");
    QNetworkRequest req = makeRequest(path);
    QJsonObject body {
        { "username", username },
        { "password", password }
    };

    auto* reply = m_nam.post(req, toJsonBytes(body));
    connect(reply, &QNetworkReply::finished, this, [this, reply, path]() {
        const auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const auto netErr = reply->error();
        const auto payload = reply->readAll();

        if (netErr != QNetworkReply::NoError || status >= 400) {
            emit loginFailed(QString::fromUtf8(payload.isEmpty() ? reply->errorString().toUtf8() : payload));
            reply->deleteLater();
            return;
        }

        const auto obj = QJsonDocument::fromJson(payload).object();
        const auto token = obj.value(QStringLiteral("token")).toString();
        if (token.isEmpty()) {
            emit loginFailed(QStringLiteral("No token in response"));
        } else {
            setToken(token);
            emit loginSucceeded(token);
        }
        reply->deleteLater();
    });
}

void gx::orm::ApiClient::get(const QString &path, const QUrlQuery &query)
{
    QNetworkRequest req = makeRequest(path, query);
    auto* reply = m_nam.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, path]() { handleReply(path, reply); });
}

void gx::orm::ApiClient::del(const QString &path, const QJsonObject &body)
{
    QNetworkRequest req = makeRequest(path);
    auto* reply = m_nam.sendCustomRequest(req, "DELETE", toJsonBytes(body));
    connect(reply, &QNetworkReply::finished, this, [this, reply, path]() { handleReply(path, reply); });
}

void gx::orm::ApiClient::post(const QString &path, const QJsonObject &body)
{
    QNetworkRequest req = makeRequest(path);
    auto* reply = m_nam.post(req, toJsonBytes(body));
    connect(reply, &QNetworkReply::finished, this, [this, reply, path]() { handleReply(path, reply); });
}

void gx::orm::ApiClient::put(const QString &path, const QJsonObject &body)
{
    QNetworkRequest req = makeRequest(path);
    auto* reply = m_nam.put(req, toJsonBytes(body));
    connect(reply, &QNetworkReply::finished, this, [this, reply, path]() { handleReply(path, reply); });
}

QNetworkRequest gx::orm::ApiClient::makeRequest(const QString &path, const QUrlQuery &query) const
{
    QUrl url = m_baseUrl;
    QString cleanPath = path.startsWith('/') ? path.mid(1) : path;
    url.setPath(url.path().endsWith('/') ? url.path() + cleanPath : url.path() + '/' + cleanPath);

    if (!query.isEmpty())
        url.setQuery(query);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("X-APP-Key", m_appKey.toUtf8());
    req.setRawHeader("X-API-Token", m_apiToken.toUtf8());
    req.setRawHeader("X-APP-Version", m_appVersion.toUtf8());

    if (!m_jwt.isEmpty())
        req.setRawHeader("Authorization", QByteArray("Bearer ").append(m_jwt.toUtf8()));

    return req;
}

void gx::orm::ApiClient::handleReply(const QString &path, QNetworkReply *reply)
{
    const auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const auto netErr = reply->error();
    const auto payload = reply->readAll();

    if (status == 401) {
        emit unauthorized(); // token missing/expired → call login or refresh
    }

    if (netErr == QNetworkReply::NoError && status < 400) {
        emit requestFinished(path, status, payload, netErr);
    } else {
        const auto msg = payload.isEmpty() ? reply->errorString() : QString::fromUtf8(payload);
        emit requestFailed(path, status == 0 ? 500 : status, msg, netErr);
    }
    reply->deleteLater();
}
