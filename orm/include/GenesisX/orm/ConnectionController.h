// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef CONNECTIONCONTROLLER_H
#define CONNECTIONCONTROLLER_H

#include <QObject>
#include <QFuture>

#include <GenesisX/Orm/genesisx_orm_global.h>

#include "TransportConfig.h"
#include "AuthCredentials.h"
#include "HttpResponse.h"
#include "HttpConfig.h"

class QNetworkAccessManager;

namespace gx::orm {

class GENESISX_ORM_EXPORT ConnectionController : public QObject
{
    Q_OBJECT

public:
    explicit ConnectionController(QObject* parent = nullptr);
    ~ConnectionController();

    void applyTransport(const TransportConfig& tcfg);

    void setConfig(const HttpConfig& cfg);
    HttpConfig config() const;

    void setAuth(const AuthCredentials& auth);
    AuthCredentials auth() const;

    void setBaseUrl(const QUrl& url);
    void setAppVersion(const QString& v);
    void setAppKey(const QString& k);
    void setApiToken(const QString& t);
    void setBearerToken(const QString& t);
    void setUserLanguage(const QString& l);
    void setDefaultHeader(const QByteArray& name, const QByteArray& value);

    QNetworkAccessManager* network() const;
    QNetworkRequest makeRequest(const QString& path) const;

    QFuture<HttpResponse> login(const QString& username, const QString& password);
    QFuture<HttpResponse> getJson(const QString& path);
    QFuture<HttpResponse> postJson(const QString& path, const QJsonObject& body);
    QFuture<HttpResponse> putJson(const QString& path, const QJsonObject& body);
    QFuture<HttpResponse> deleteJson(const QString& path);

    bool hasBearer() const;

signals:
    void authChanged();
    void appVersionChanged();
    void loginSucceeded(QString token);
    void loginFailed(QString error);

private:
    struct Impl;
    Impl* d;
};

}

#endif // CONNECTIONCONTROLLER_H
