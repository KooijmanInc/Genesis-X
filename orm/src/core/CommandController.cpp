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
    \class gx::orm::CommandController
    \inheaderfile ../../../include/GenesisX/Orm/CommandController.h
    \inmodule GenesisX
    \ingroup orm-classes
    \title CommandController for api and database
    \since 6.10
    \brief Use CommandController for getting database record via sql connection or api.
 */

/*!
    \qmlmodule GenesisX.CommandController
    \inqmlmodule io.genesisx.orm
    \title Genesis-X CommandController (QML)
    \since Qt 6.10
    \nativetype gx::orm::CommandController
    \brief QML APIs for setting commands to remote API via json or database.
 */

/*!
    \qmltype CommandController
    \inqmlmodule io.genesisx.orm
    \since Qt 6.10
    \nativetype gx::orm::CommandController
    \brief Handles API and database commands.
 */

/*!
    \qmlsignal CommandController::requestFinished(string path, bool ok, int status, string error, object obj)

    Emitted when the postrequest got response.
    The \a path where the request was send to.
    The \a ok boolean if it was successful.
    A http response \a status e.g. 200 or 400.
    The \a error shows what it wrong when getting other than status 200
    The return body that must be in an JsonObject \a obj.
*/

using namespace gx::orm;

namespace gx::orm {

CommandController::CommandController(ConnectionController *conn, QObject *parent)
    : QObject{parent}
    , m_conn(conn)
{
}

/*!
    \qmlmethod void CommandController::cmdLogin(string user, string pass)

    Set login credentials for JWT authorization, add \a user as username and \a pass as password
 */
void CommandController::cmdLogin(const QString &user, const QString &pass)
{
    auto fut = m_conn->login(user, pass);
    fut.then([](const HttpResponse& r) {
        qInfo() << "[GX cmdLogin]" << r.status << QString::fromUtf8(r.body.left(200)) << r.errorstring;
    });
}

/*!
    \qmlmethod void CommandController::cmdPing()

    Returns a signal if connection to remote server is successful
 */
void CommandController::cmdPing()
{
    auto fut = m_conn->getJson("ping");
    fut.then([](const HttpResponse& r){
        qInfo() << "[cmdPing]" << r.status << QString::fromUtf8(r.body) << r.errorstring;
    });
}

/*!
    \qmlmethod CommandController::cmdRegister(string email, string password)

    A Stub to check if postJson is correct, testing the connection
    Send \a email and \a password for testing
 */
void CommandController::cmdRegister(const QString &email, const QString &password)
{
    QJsonObject body{{"email", email}, {"password", password}};
    auto fut = m_conn->postJson("register", body);
    fut.then([](const HttpResponse& r){
        qInfo() << "[cmdRegister]" << r.status << QString::fromUtf8(r.body.left(200)) << r.errorstring;
    });
}

/*!
    \qmlmethod CommandController::cmdPostJson(string path, var vm)
    Send \a path as the path in url for connecting to API function
    Send \a vm as the QVariantMap as parameters you need at that url request e.g. Customernumber 1, Address Krijtlaan etc.
 */
void CommandController::cmdPostJson(const QString &path, const QVariantMap& vm)
{
    const QJsonObject body = QJsonObject::fromVariantMap(vm);
    (void)cmdPostJsonAsync(path, body);
}

/*!
    \qmlmethod CommandController::cmdPostJsonBoolAsync(string path, object body)
    Send \a path as the path in url for connecting to API function
    Send \a body as a QJsonObject, it returns only a boolean if post was successful
 */
QFuture<bool> CommandController::cmdPostJsonBoolAsync(const QString &path, const QJsonObject &body)
{
    return cmdPostJsonAsync(path,body).then([](const HttpResponse& r){
        return r.ok();
    });
}

/*!
    \qmlmethod CommandController::cmdPostJsonBoolAsync(string path, object body)
    Send \a path as the path in url for connecting to API function
    Send \a body as a QJsonObject.
    Its the same as cmdPostJsonBoolAsync but it returns HttpResponse with body
 */
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
