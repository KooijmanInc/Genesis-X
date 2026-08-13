// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <QDebug>

#include <GenesisX/Orm/AbstractRepository.h>

using namespace gx::orm::repository;

AbstractRepository::AbstractRepository(QObject *parent)
    : QObject{parent}
{
    qDebug() << "abstract repository";
}
