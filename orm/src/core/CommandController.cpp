// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/CommandController.h>

#include <GenesisX/Orm/ConnectionController.h>
#include <GenesisX/Orm/HttpResponse.h>

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/*!
    \namespace gx::orm
    \inmodule GenesisX.Orm
    \title gx::orm Namespace
    \brief Orm facilities.
 */

/*!
    \class gx::orm::CommandController
    \inheaderfile GenesisX/Orm/CommandController.h
    \inmodule GenesisX.Orm
    \ingroup genesisx-orm
    \title CommandController for api and database
    \since 6.10
    \brief Use CommandController for getting database record via sql connection or api.
 */

using namespace gx::orm;

namespace gx::orm {

CommandController::CommandController(ConnectionController *conn, QObject *parent)
    : QObject{parent}
    , m_conn(conn)
{
}

void CommandController::cmdLogin(const QString &user, const QString &pass)
{
    auto fut = m_conn->login(user, pass);
    fut.then([](const HttpResponse& r) {
        qInfo() << "[GX cmdLogin]" << r.status << QString::fromUtf8(r.body.left(200)) << r.errorstring;
    });
}

void CommandController::cmdPing()
{
    auto fut = m_conn->getJson("ping");
    fut.then([](const HttpResponse& r){
        qInfo() << "[cmdPing]" << r.status << QString::fromUtf8(r.body) << r.errorstring;
    });
}

void CommandController::cmdRegister(const QString &email, const QString &password)
{
    QJsonObject body{{"email", email}, {"password", password}};
    auto fut = m_conn->postJson("register", body);
    fut.then([](const HttpResponse& r){
        qInfo() << "[cmdRegister]" << r.status << QString::fromUtf8(r.body.left(200)) << r.errorstring;
    });
}

void CommandController::cmdPostJson(const QString &path, const QVariantMap& vm)
{
    const QJsonObject body = QJsonObject::fromVariantMap(vm);
    (void)cmdPostJsonAsync(path, body);
}

QFuture<bool> CommandController::cmdPostJsonBoolAsync(const QString &path, const QJsonObject &body)
{
    return cmdPostJsonAsync(path,body).then([](const HttpResponse& r){
        return r.ok();
    });
}

QFuture<HttpResponse> CommandController::cmdPostJsonAsync(const QString &path, const QJsonObject &body)
{
    return m_conn->postJson(path, body).then([this, path](const HttpResponse& r) {
        const bool ok = r.ok();
        QMetaObject::invokeMethod(this, [this, path, ok, r]() {
            QString perr;
            emit requestFinished(path, ok, r.status, r.errorstring, r.jsonObject(&perr));
        }, Qt::QueuedConnection);
        return r;
    });
}

}
