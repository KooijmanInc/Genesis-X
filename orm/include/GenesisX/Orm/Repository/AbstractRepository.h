// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef ABSTRACTREPOSITORY_H
#define ABSTRACTREPOSITORY_H

#include <GenesisX/Orm/Core/genesisx_orm_global.h>

#include <GenesisX/Orm/Entity/AbstractEntity.h>

namespace gx::orm {

class GENESISX_ORM_EXPORT AbstractRepository
{
public:
    AbstractRepository() = default;
    virtual ~AbstractRepository() = default;

    [[nodiscard]] virtual AbstractEntity *create() = 0;

protected:
    void attach(AbstractEntity* entity)
    {
        entity->setRepository(this);
    }
};

}

#endif // ABSTRACTREPOSITORY_H
