// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef ABSTRACTREPOSITORY_H
#define ABSTRACTREPOSITORY_H

#include <QObject>

#include <GenesisX/Orm/genesisx_orm_global.h>

namespace gx::orm::repository {

class GENESISX_ORM_EXPORT AbstractRepository : public QObject
{
    Q_OBJECT

public:
    explicit AbstractRepository(QObject* parent = nullptr);
    ~AbstractRepository() override = default;
};

}

#endif // ABSTRACTREPOSITORY_H
