#ifndef COMMANDCONTROLLER_H
#define COMMANDCONTROLLER_H

#include <QObject>

#include "genesisx_orm_global.h"

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

private:
    gx::orm::ConnectionController* m_conn;
};

}

#endif // COMMANDCONTROLLER_H
