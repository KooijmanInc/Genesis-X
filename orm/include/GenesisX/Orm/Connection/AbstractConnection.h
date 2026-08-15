// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef ABSTRACTCONNECTION_H
#define ABSTRACTCONNECTION_H

#include <QObject>
#include <QFuture>

#include <GenesisX/Orm/Core/genesisx_orm_global.h>
#include <GenesisX/Orm/Connection/ConnectionResult.h>

namespace gx::orm {

class GENESISX_ORM_EXPORT AbstractConnection : public QObject
{
    Q_OBJECT

public:
    explicit AbstractConnection(QObject* parent = nullptr);
    ~AbstractConnection() override = default;

    virtual QFuture<ConnectionResult> ping() = 0;
};

}


#endif // ABSTRACTCONNECTION_H
