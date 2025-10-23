#include "include/GenesisX/Orm/CommandController.h"

#include <GenesisX/Orm/ConnectionController.h>
#include <GenesisX/Orm/HttpResponse.h>

#include <QJsonObject>

using namespace gx::orm;

CommandController::CommandController(ConnectionController *conn, QObject *parent)
    : QObject{parent}
    , m_conn(conn)
{
}

void CommandController::cmdLogin(const QString &user, const QString &pass)
{
    auto fut = m_conn->login(user, pass);
    fut.then([](const HttpResponse& r) {
        qInfo() << "[GX cmdLogin]" << r.status << QString::fromUtf8(r.body.left(200));
    });
}

void CommandController::cmdPing()
{
    auto fut = m_conn->getJson("ping");
    fut.then([](const HttpResponse& r){
        qInfo() << "[cmdPing]" << r.status << QString::fromUtf8(r.body);
    });
}

void CommandController::cmdRegister(const QString &email, const QString &password)
{
    QJsonObject body{{"email", email}, {"password", password}};
    auto fut = m_conn->postJson("register", body);
    fut.then([](const HttpResponse& r){
        qInfo() << "[cmdRegister]" << r.status << QString::fromUtf8(r.body.left(200));
    });
}
