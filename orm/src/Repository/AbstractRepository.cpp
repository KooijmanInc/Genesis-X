// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <QDebug>

#include <GenesisX/Orm/Repository/AbstractRepository.h>

#include <GenesisX/Orm/Connection/ConnectionFactory.h>
#include <GenesisX/Orm/Connection/AbstractConnection.h>

using namespace gx::orm;

AbstractRepository::AbstractRepository(QObject *parent)
    : QObject{parent}
{
    qDebug() << "abstract repository";
}

QFuture<ConnectionResult> AbstractRepository::ping()
{
    AbstractConnection *activeConnection =
        connection();

    if (activeConnection == nullptr) {
        const auto &factory =
            ConnectionFactory::instance();

        return QtFuture::makeReadyValueFuture(
            ConnectionResult{
                .backend = factory.backend(),
                .successful = false,
                .message = QStringLiteral(
                    "No connection configured"
                    ),
            }
            );
    }

    return activeConnection->ping();
}

AbstractConnection *AbstractRepository::connection() const
{
    return ConnectionFactory::instance().connection();
}
