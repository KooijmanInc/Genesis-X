// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/Connection/HttpConnection.h>
#include <GenesisX/Orm/Connection/HttpRequestState.h>

#include <QtConcurrent>
#include <QMultiHash>

// #include <QElapsedTimer>
// #include <QNetworkReply>
// #include <QNetworkRequest>
// #include <QPromise>
// #include <QTimer>

using namespace gx::orm;


gx::orm::HttpConnection::HttpConnection(const HttpConfig &config, QObject *parent)
    : AbstractConnection{parent}
    , m_config{config}
{}

QFuture<ConnectionResult> gx::orm::HttpConnection::ping()
{
    return get(QStringLiteral("ping"))
        .then([](const HttpResponse &response) {
            return ConnectionResult{
                .backend = Backend::Http,
                .successful = response.ok(),
                .latencyMs = response.latencyMs,
                .statusCode = response.statusCode,
                .message = response.ok()
                ? QStringLiteral("HTTP connection successful")
                : response.errorString,
            };
        });
}

// QFuture<ConnectionResult> HttpConnection::execute(const QString &statement, const QVariantMap &bindings)
// {
//     return QtConcurrent::run(
//         [statement, bindings]() -> ConnectionResult
//         {
//             return {
//                 .successful = true,
//                 .statusCode = 0,
//                 .message = "HTTP execution successful"
//             };
//         });
// }

QUrl HttpConnection::resolveUrl(const QString &path) const
{
    if (!m_config.baseUrl.isValid()) {
        return {};
    }

    QUrl baseUrl = m_config.baseUrl;
    QString basePath = baseUrl.path();

    if (basePath.isEmpty()) {
        basePath = QStringLiteral("/");
    } else if (!basePath.endsWith('/')) {
        basePath += '/';
    }

    baseUrl.setPath(basePath);

    QString relativePath = path;

    while (relativePath.startsWith('/')) {
        relativePath.removeFirst();
    }

    return baseUrl.resolved(QUrl{relativePath});
}

QFuture<HttpResponse> HttpConnection::get(const QString &path)
{
    auto state = std::make_shared<HttpRequestState>();

    QFuture<HttpResponse> future = state->promise.future();
    state->promise.start();

    const QUrl url = resolveUrl(path);

    if (!url.isValid()) {
        state->promise.addResult(HttpResponse{
            .body = "",
            .errorString = QStringLiteral("Invalid request URL"),
        });

        state->promise.finish();
        return future;
    }

    QNetworkRequest request{url};

    request.setRawHeader(
        "Accept",
        QByteArrayLiteral("application/json")
        );

    request.setAttribute(
        QNetworkRequest::RedirectPolicyAttribute,
        QNetworkRequest::NoLessSafeRedirectPolicy
        );

    QNetworkReply *reply =
        m_networkAccessManager.get(request);

    state->elapsedTimer.start();

    auto *timeoutTimer = new QTimer{reply};
    timeoutTimer->setSingleShot(true);

    const int timeoutMs =
        m_config.timeoutMs > 0
            ? m_config.timeoutMs
            : 15000;

    connect(
        timeoutTimer,
        &QTimer::timeout,
        reply,
        [reply, state]() {
            state->timedOut = true;
            reply->abort();
        }
        );

    connect(
        reply,
        &QNetworkReply::finished,
        reply,
        [reply, timeoutTimer, timeoutMs, state]() {
            timeoutTimer->stop();

            HttpResponse response;

            response.latencyMs =
                static_cast<int>(
                    state->elapsedTimer.elapsed()
                    );

            response.networkError = reply->error();
            response.body = reply->readAll();

            const QVariant status =
                reply->attribute(
                    QNetworkRequest::HttpStatusCodeAttribute
                    );

            if (status.isValid()) {
                response.statusCode = status.toInt();
            }

            for (const auto &[name, value]
                 : reply->rawHeaderPairs()) {
                response.headers.insert(name, value);
            }

            if (state->timedOut) {
                response.errorString =
                    QStringLiteral(
                        "Request timed out after %1 ms"
                        ).arg(timeoutMs);
            } else if (
                response.networkError
                != QNetworkReply::NoError
                ) {
                response.errorString =
                    reply->errorString();
            }

            state->promise.addResult(
                std::move(response)
                );

            state->promise.finish();

            reply->deleteLater();
        }
        );

    timeoutTimer->start(timeoutMs);

    return future;
}
