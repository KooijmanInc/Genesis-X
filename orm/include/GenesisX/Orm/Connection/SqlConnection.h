// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef SQLCONNECTION_H
#define SQLCONNECTION_H

#include <QSqlDatabase>

#include <GenesisX/Orm/Connection/AbstractConnection.h>

namespace gx::orm {

class GENESISX_ORM_EXPORT SqlConnection final
    : public AbstractConnection
{
    Q_OBJECT

public:
    explicit SqlConnection(
        const SqlConfig &config,
        QObject *parent = nullptr
        );
    ~SqlConnection() override;

    QFuture<ConnectionResult> ping() override;

    QSqlDatabase database() const;

private:
    void configureDatabase();

    SqlConfig m_config;
    QString m_connectionName;
    QSqlDatabase m_database;
};

}

#endif // SQLCONNECTION_H
