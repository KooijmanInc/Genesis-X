// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef COMMANDCONTROLLER_H
#define COMMANDCONTROLLER_H

#include <QObject>
#include <QFuture>
#include <QJsonObject>

#include <GenesisX/Orm/genesisx_orm_global.h>

#include <GenesisX/Orm/HttpResponse.h>

/*!
    \namespace gx::orm
    \inmodule GenesisX.Orm
    \title gx::orm Namespace
    \brief Module-level APIs.
 */
namespace gx::orm {

class ConnectionController;

class GENESISX_ORM_EXPORT CommandController : public QObject
{
    Q_OBJECT

public:
    explicit CommandController(ConnectionController* conn, QObject* parent = nullptr);

    Q_INVOKABLE void cmdLogin(const QString& user, const QString& pass);
    Q_INVOKABLE void cmdPing();
    Q_INVOKABLE void cmdRegister(const QString& email, const QString& password);
    Q_INVOKABLE void cmdPostJson(const QString& path, const QVariantMap& vm);

    QFuture<bool> cmdPostJsonBoolAsync(const QString& path, const QJsonObject& body);
    QFuture<HttpResponse> cmdPostJsonAsync(const QString& path, const QJsonObject& body);

signals:
    void requestFinished(QString path, bool ok, int status, QString error, QJsonObject obj);

private:
    gx::orm::ConnectionController* m_conn;
};

}

#endif // COMMANDCONTROLLER_H
