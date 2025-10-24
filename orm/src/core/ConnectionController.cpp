// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/ConnectionController.h>
#include <GenesisX/Orm/AuthCredentials.h>
#include <GenesisX/Orm/HttpResponse.h>
#include <GenesisX/Orm/HttpConfig.h>

#include <QLoggingCategory>

#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QJsonObject>
#include <QPointer>
#include <QTimer>

using namespace gx::orm;

static inline QUrl resolvePath(const QUrl& base, const QString& path)
{
    if (!base.isValid()) return {};
    QUrl baseFixed = base;

    if (!baseFixed.path().endsWith('/')) {
        QString p = baseFixed.path();
        if (p.isEmpty()) p = "/";
        else if (!p.endsWith('/')) p += '/';
        baseFixed.setPath(p);
    }

    return baseFixed.resolved(QUrl(path));
}

static inline void debugConnections(auto reply)
{
    QLoggingCategory::setFilterRules(
        "qt.network.access.debug=true\n"
        "qt.network.ssl.warning=true\n"
        );

    QObject::connect(reply, &QNetworkReply::sslErrors, reply,
                     [](const QList<QSslError>& errs){
                         qDebug() << "[HTTP] sslErrors" << errs;
                     });
    QObject::connect(reply, &QNetworkReply::errorOccurred, reply,
                     [reply](QNetworkReply::NetworkError e){
                         qDebug() << "[HTTP] errorOccurred" << e << reply->errorString();
                     });
    QObject::connect(reply, &QIODevice::bytesWritten, reply,
                     [](qint64 n){ qDebug() << "[HTTP] bytesWritten" << n; });

    QObject::connect(reply, &QNetworkReply::uploadProgress, reply,
                     [](qint64 sent, qint64 total){
                         qDebug() << "[HTTP] uploadProgress" << sent << "/" << total;
                     });

    QObject::connect(reply, &QNetworkReply::finished, reply, []{
        qDebug() << "[HTTP] finished() emitted";
    });
    QObject::connect(reply, &QNetworkReply::readyRead, reply, []{
        qDebug() << "[HTTP] readyRead()";
    });

    QObject::connect(reply, &QNetworkReply::metaDataChanged, reply, [reply]{
        const auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "[HTTP] metaDataChanged status" << status
                 << "reason" << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toByteArray();
    });

    QObject::connect(reply, &QNetworkReply::readyRead, reply, [reply]{
        qDebug() << "[HTTP] readyRead chunk" << reply->peek(256); // peek small chunk
    });
}

static inline void applyHeaders(QNetworkRequest& req, const HttpConfig& cfg, const AuthCredentials& auth)
{
    if (!req.hasRawHeader("Accept")) {
        req.setRawHeader("Accept", "application/json");
    }
    // Let Content-Type be set by the specific verb, but default to JSON:
    if (!req.hasRawHeader("Content-Type")) {
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    }

    if (!cfg.appVersion.isEmpty()) {
        req.setRawHeader("X-App-Version", cfg.appVersion.toUtf8());
    }
    if (!cfg.userLanguage.isEmpty()) {
        req.setRawHeader("Accept-Language", cfg.userLanguage.toUtf8());
    }
    if (!auth.appKey.isEmpty()) {
        req.setRawHeader("X-App-Key", auth.appKey.toUtf8());
    }
    if (!auth.apiToken.isEmpty()) {
        req.setRawHeader("X-Api-Token", auth.apiToken.toUtf8());
    }
    if (!auth.bearerToken.isEmpty()) {
        req.setRawHeader("Authorization", QByteArray("Bearer ").append(auth.bearerToken.toUtf8()));
    }

    // user defaults last (allows override)
    for (auto it = cfg.defaultHeaders.constBegin(); it != cfg.defaultHeaders.constEnd(); ++it) {
        req.setRawHeader(it.key(), it.value());
    }

    // follow safe redirects by default
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
}

struct ConnectionController::Impl {
    HttpConfig cfg;
    AuthCredentials auth;
    QNetworkAccessManager nam;
};
QNetworkAccessManager* ConnectionController::network() const { return &d->nam; }

ConnectionController::ConnectionController(QObject *parent)
    : QObject{parent}
    , d(new Impl)
{

}

ConnectionController::~ConnectionController()
{
    delete d;
}

void ConnectionController::setConfig(const HttpConfig &cfg)
{
    d->cfg = cfg;
}

HttpConfig ConnectionController::config() const
{
    return d->cfg;
}

void ConnectionController::setAuth(const AuthCredentials &auth)
{
    const bool verChanged = (d->auth.bearerToken != auth.bearerToken) || (d->auth.appKey != auth.appKey) || (d->auth.apiToken != auth.apiToken);
    d->auth = auth;
    if (verChanged)
        emit authChanged();
}

AuthCredentials ConnectionController::auth() const
{
    return d->auth;
}

void ConnectionController::setBaseUrl(const QUrl &url)
{
    d->cfg.baseUrl = url;
}

void ConnectionController::setAppVersion(const QString &v)
{
    if (d->cfg.appVersion == v) return;
    d->cfg.appVersion = v;
    emit appVersionChanged();
}

void ConnectionController::setApiToken(const QString &t)
{
    if (d->auth.apiToken == t) return;
    d->auth.apiToken = t;
    emit authChanged();
}

void ConnectionController::setAppKey(const QString &k)
{
    if (d->auth.appKey == k) return;
    d->auth.appKey = k;
    emit authChanged();
}

void ConnectionController::setBearerToken(const QString &t)
{
    if (d->auth.bearerToken == t) return;
    d->auth.bearerToken = t;
    emit authChanged();
}

void ConnectionController::setUserLanguage(const QString &l)
{
    if (d->cfg.userLanguage == l) return;
    d->cfg.userLanguage = l;
}

void ConnectionController::setDefaultHeader(const QByteArray &name, const QByteArray &value)
{
    d->cfg.defaultHeaders.insert(name, value);
}


QNetworkRequest ConnectionController::makeRequest(const QString& path) const
{
    const QUrl url = resolvePath(d->cfg.baseUrl, path);

    QNetworkRequest req(url);
    applyHeaders(req, d->cfg, d->auth);
#if QT_VERSION <= QT_VERSION_CHECK(6, 5, 0)
    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);
#endif

    return req;
}

template <typename StartFn>
static QFuture<HttpResponse> runWithTimeout(QNetworkAccessManager* nam, const HttpConfig& cfg, const QNetworkRequest& req, StartFn&& start)
{
    QPromise<HttpResponse> promise;
    auto future = promise.future();
    promise.start();

    if (!req.url().isValid()) {
        HttpResponse r;
        r.errorstring = QStringLiteral("Invalid URL");
        promise.addResult(r);
        promise.finish();

        return future;
    }

    QPointer<QNetworkAccessManager> namPtr(nam);
    QNetworkReply* reply = start(nam, req);
    QPointer<QNetworkReply> replyPtr(reply);

    QList<QSslError> sslErrs;
    QObject::connect(reply, &QNetworkReply::sslErrors, reply, [&](const QList<QSslError>& errs){
        sslErrs = errs;

        const QString h = reply->url().host();
        const bool isLocalHostLike = h.endsWith(".localhost", Qt::CaseInsensitive) || h == "localhost" || h == "127.0.0.1";
        if (cfg.allowInsecureDev && isLocalHostLike) {
            reply->ignoreSslErrors(errs);
            debugConnections(reply);
        }
    });

    const int timeoutMs = (cfg.timeoutMs > 0) ? cfg.timeoutMs : 15000;
    QTimer* timer = new QTimer(reply);
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, reply, [reply](){ reply->abort(); });
    timer->start(timeoutMs);

    QObject::connect(reply, &QNetworkReply::finished, reply, [reply, sslErrs, promise = std::move(promise)]() mutable {
        HttpResponse r;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        r.netError = reply->error();
#else
        r.netError = reply->error();
#endif
        r.body = reply->readAll();

        const auto statusAttr = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (statusAttr.isValid()) r.status = statusAttr.toInt();
        const auto reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toByteArray();
        if (!reason.isEmpty() && r.errorstring.isEmpty())
            r.errorstring = QString::fromLatin1(reason);

        for (const auto& h : reply->rawHeaderPairs())
            r.headers.insert(h.first, h.second);

        r.sslErrors = sslErrs;

        if (r.errorstring.isEmpty() && r.netError != QNetworkReply::NoError)
            r.errorstring = reply->errorString();

        reply->deleteLater();
        promise.addResult(r);
        promise.finish();
    });


    return future;
}

QFuture<HttpResponse> ConnectionController::getJson(const QString &path)
{
    QNetworkRequest req = makeRequest(path);
    return runWithTimeout(&d->nam, d->cfg, req, [](QNetworkAccessManager* nam, const QNetworkRequest& r) {
        return nam->get(r);
    });
}

QFuture<HttpResponse> ConnectionController::login(const QString &username, const QString &password)
{
    const QString path = QStringLiteral("login_check");

    QNetworkRequest req = makeRequest(path);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    const QJsonObject body {
        { QStringLiteral("username"), username },
        { QStringLiteral("password"), password }
    };

    const QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);

    return runWithTimeout(&d->nam, d->cfg, req, [payload](QNetworkAccessManager* nam, const QNetworkRequest& r) {
               return nam->post(r, payload);
           }).then([this](const HttpResponse& r) -> HttpResponse {
            // On success (2xx), try to parse { "token": "<JWT>" } and store it
            if (r.status >= 200 && r.status < 300) {
                QJsonParseError jerr{};
                const QJsonDocument doc = QJsonDocument::fromJson(r.body, &jerr);
                if (jerr.error == QJsonParseError::NoError && doc.isObject()) {
                    const auto tok = doc.object().value(QStringLiteral("token")).toString();
                    if (!tok.isEmpty()) {
                        setBearerToken(tok);
                        emit loginSucceeded(tok);
                        return r;
                    }
                }
                // Token missing even though 2xx
                emit loginFailed(QStringLiteral("No token in login response"));
            } else {
                // Pass through server error body if available
                const QString errText = r.body.isEmpty()
                                            ? QStringLiteral("Login failed (status %1)").arg(r.status)
                                            : QString::fromUtf8(r.body);
                emit loginFailed(errText);
            }
            return r;
        });
}

QFuture<HttpResponse> ConnectionController::postJson(const QString &path, const QJsonObject &body)
{
    QNetworkRequest req = makeRequest(path);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    const QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
    qInfo() << "[HTTP] POST" << req.url().toString()
            << "len" << payload.size()
            << "headers:"
            << req.rawHeaderList();

    return runWithTimeout(&d->nam, d->cfg, req, [payload](QNetworkAccessManager* nam, const QNetworkRequest& r) {
        auto reply = nam->post(r, payload);
        return reply;
    });
}

QFuture<HttpResponse> ConnectionController::putJson(const QString &path, const QJsonObject &body)
{
    QNetworkRequest req = makeRequest(path);
    const QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return runWithTimeout(&d->nam, d->cfg, req, [payload](QNetworkAccessManager* nam, const QNetworkRequest& r) {
        return nam->put(r, payload);
    });
#else
    return runWithTimeout(&d->nam, d->cfg, req, [payload](QNetworkAccessManager* nam, const QNetworkRequest& r) {
        return nam->sendCustomRequest(r, "PUT", payload);
    });
#endif
}

QFuture<HttpResponse> ConnectionController::deleteJson(const QString &path)
{
    QNetworkRequest req = makeRequest(path);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return runWithTimeout(&d->nam, d->cfg, req, [](QNetworkAccessManager* nam, const QNetworkRequest& r) {
        return nam->deleteResource(r);
    });
#else
    return runWithTimeout(&d->nam, d->cfg, req, [](QNetworkAccessManager* nam, const QNetworkRequest& r) {
        return nam->deleteResource(r);
    });
#endif
}

bool ConnectionController::hasBearer() const
{
    return !d->auth.bearerToken.isEmpty();
}
