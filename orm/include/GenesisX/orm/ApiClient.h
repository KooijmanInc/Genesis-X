#ifndef APICLIENT_H
#define APICLIENT_H

#include <QUrl>
#include <QObject>
#include <QSettings>
#include <QUrlQuery>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include "genesisx_orm_global.h"

namespace gx::orm {

class GENESISX_ORM_EXPORT ApiClient : public QObject
{
    Q_OBJECT

public:
    explicit ApiClient(const QUrl& baseUrl, QString appKey, QString apiToken, QString appVersion, QObject* parent = nullptr);

    void setAppKey(QString v) { m_appKey = std::move(v); }
    void setApiToken(QString v) { m_apiToken = std::move(v); }
    void setAppVersion(QString v) { m_appVersion = std::move(v); }

    QString token() const { return m_jwt; }
    void setToken(QString t);

    void setTokenPersistance(bool enabled, QString org = {}, QString app = {}, QString key = QStringLiteral("auth/jwt"));

    Q_INVOKABLE void login(const QString& username, const QString& password);

    Q_INVOKABLE void get (const QString& path, const QUrlQuery& query = {});
    Q_INVOKABLE void del (const QString& path, const QJsonObject& body = {});
    Q_INVOKABLE void post(const QString& path, const QJsonObject& body = {});
    Q_INVOKABLE void put (const QString& path, const QJsonObject& body = {});

signals:
    void loginFailed(QString error);
    void loginSucceeded(QString jwt);

    void unauthorized();
    void requestFinished(QString path, int status, QByteArray payload, QNetworkReply::NetworkError error);
    void requestFailed(QString path, int status, QString error, QNetworkReply::NetworkError netError);

private:
    QNetworkRequest makeRequest(const QString& path, const QUrlQuery& query = {}) const;
    void sendJson(const QString& method, const QString& path, const QJsonObject& body, const QUrlQuery& query = {});
    void handleReply(const QString& path, QNetworkReply* reply);

private:
    QNetworkAccessManager m_nam;
    QUrl m_baseUrl;
    QString m_appKey, m_apiToken, m_appVersion;
    QString m_jwt;

    bool m_persist = false;
    QString m_org, m_app, m_storeKey;
};

}

#endif // APICLIENT_H
