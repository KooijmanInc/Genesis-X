// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef ABSTRACTREPOSITORY_H
#define ABSTRACTREPOSITORY_H

#include <QFuture>
#include <QObject>

#include <GenesisX/Orm/Core/genesisx_orm_global.h>

#include <GenesisX/Orm/Connection/ConnectionResult.h>

#include <GenesisX/Orm/Entity/AbstractEntity.h>

namespace gx::orm {

class AbstractConnection;

class GENESISX_ORM_EXPORT AbstractRepository : public QObject
{
    Q_OBJECT

public:
    AbstractRepository(QObject* parent = nullptr);
    ~AbstractRepository() override = default;

    QFuture<ConnectionResult> ping();

    [[nodiscard]] virtual AbstractEntity *create() = 0;

protected:
    AbstractConnection* connection() const;

    void attach(AbstractEntity* entity)
    {
        entity->setRepository(this);
    }
};

}

#endif // ABSTRACTREPOSITORY_H
