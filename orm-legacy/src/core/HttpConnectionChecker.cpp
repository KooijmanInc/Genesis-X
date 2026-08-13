// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "include/GenesisX/Orm/ConnectionCheck.h"

#include <GenesisX/Orm/ConnectionController.h>
#include <GenesisX/Orm/HttpConfig.h>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QTimer>

using namespace gx::orm;

static inline void applyHeaders(QNetworkRequest& req, const HttpConfig& cfg, const AuthCredentials& auth)
{
    if (!req.hasRawHeader("Accept")) req.setRawHeader("Accept", "application/json");
    if (!req.hasRawHeader("Content-Type")) req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (!cfg.appVersion.isEmpty())
        req.setRawHeader("x-app-version", cfg.appVersion.toUtf8());

    if (!auth.appKey.isEmpty())
        req.setRawHeader("x-app-key", auth.appKey.toUtf8());
    if (!auth.bearerToken.isEmpty())
        req.setRawHeader("Authorization", QByteArray("Bearer ").append(auth.bearerToken.toUtf8()));

    for (auto it = cfg.defaultHeaders.constBegin(); it != cfg.defaultHeaders.constEnd(); ++it)
        req.setRawHeader(it.key(), it.value());
}

static inline QUrl resolvePath(const QUrl& base, const QString& path)
{
    if (!base.isValid()) return {};

    QUrl baseFixed = base;
    if (!baseFixed.path().endsWith("/")) {
        QString p = baseFixed.path();
        if (p.isEmpty()) p = "/";
        else if (!p.endsWith("/")) p += '/';
        baseFixed.setPath(p);
    }

    return baseFixed.resolved(QUrl(path));
}

HttpConnectionChecker::HttpConnectionChecker(ConnectionController *conn)
    : m_conn{conn}
{
}

HttpConnectionChecker::~HttpConnectionChecker() = default;

static GxConnectionReport makeFailure(const QString msg)
{
    GxConnectionReport r;
    r.backend = QStringLiteral("http");
    r.ok = false;
    r.latencyMs = -1;
    r.message = msg;

    return r;
}

GxConnectionReport HttpConnectionChecker::check()
{
    if (!m_conn)
        return makeFailure(QStringLiteral("No ConnectionController"));

    const HttpConfig cfg = m_conn->config();
    if (!cfg.baseUrl.isValid())
        return makeFailure(QStringLiteral("Invalid baseUrl"));

    const QUrl healthUrl = resolvePath(cfg.baseUrl, QStringLiteral("health"));
    const QUrl baseUrl = resolvePath(cfg.baseUrl, QStringLiteral(""));

    QNetworkAccessManager* nam = m_conn->network();
    QElapsedTimer timer;
    QEventLoop loop;
    QTimer timeout;

    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);

    auto doRequest = [&](const QNetworkRequest& req, const char* method) -> std::pair<int, QString> {
        timer.restart();
        QNetworkReply* reply = nam->head(req);

        if (qstrcmp(method, "HEAD") == 0) {
            reply = nam->head(req);
        } else {
            reply = nam->get(req);
        }

        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

        int effectiveTimeout = cfg.timeoutMs > 0 ? cfg.timeoutMs : 15000;
        timeout.start(effectiveTimeout);
        loop.exec();

        int elapsed = static_cast<int>(timer.elapsed());

        if (!reply->isFinished()) {
            reply->abort();
            reply->deleteLater();
            timeout.stop();

            GxConnectionReport out;
            out.backend = QStringLiteral("http");
            out.ok = false;
            out.latencyMs = elapsed;
            out.message = QStringLiteral("Timeout after %1 ms").arg(effectiveTimeout);
            return { -1, out.message };
        }

        timeout.stop();

        const QVariant statusAttr = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        const int status = statusAttr.isValid() ? statusAttr.toInt() : 0;

        QString msg;
        if (status >= 200 && status <= 299) {
            msg = QStringLiteral("%1 %2 OK").arg(QString::fromLatin1(method)).arg(status);
        } else {
            msg = QStringLiteral("%1 %2 %3")
                    .arg(QString::fromLatin1(method))
                    .arg(status)
                      .arg(QString::fromLatin1(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toByteArray()));
        }

        if (status == 0) {
            const auto err = reply->errorString();
            msg = QStringLiteral("%1 0 (%2)").arg(QString::fromLatin1(method), err);
        }

        reply->deleteLater();
        return { status, msg };
    };

    auto makeRequestFor = [&](const QUrl& url) {
        QNetworkRequest req(url);
        applyHeaders(req, cfg, m_conn->auth());

        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
        return req;
    };

    {
        QUrl url = healthUrl.isValid() ? healthUrl : baseUrl;
        QNetworkRequest req = makeRequestFor(url);

        auto [ status, msg ] = doRequest(req, "HEAD");

        if (status >= 200 && status <= 299) {
            GxConnectionReport out;
            out.backend = QStringLiteral("http");
            out.ok = true;
            out.latencyMs = static_cast<int>(timer.elapsed());
            out.message = msg;

            return out;
        }

        if (status == 405 || status == 404 || status == -1) {
            // fallthrough to GET below
        } else if (status == 401 || status == 403) {
            GxConnectionReport out;
            out.backend   = QStringLiteral("http");
            out.ok        = false;
            out.latencyMs = static_cast<int>(timer.elapsed());
            out.message   = QStringLiteral("Auth failed: %1").arg(msg);
            return out;
        } else if (status != 0) {
            // Other non-2xx: still try GET as a second chance
        }
    }

    {
        QUrl url = healthUrl.isValid() ? healthUrl : baseUrl;
        QNetworkRequest req = makeRequestFor(url);
        auto [status, msg] = doRequest(req, "GET");

        GxConnectionReport out;
        out.backend   = QStringLiteral("http");
        out.latencyMs = static_cast<int>(timer.elapsed());

        if (status >= 200 && status <= 299) {
            out.ok = true;
            out.message = msg;
        } else if (status == -1) {
            out.ok = false;
            out.message = QStringLiteral("Timeout");
        } else if (status == 401 || status == 403) {
            out.ok = false;
            out.message = QStringLiteral("Auth failed: %1").arg(msg);
        } else {
            out.ok = false;
            out.message = msg;
        }
        return out;
    }
}
