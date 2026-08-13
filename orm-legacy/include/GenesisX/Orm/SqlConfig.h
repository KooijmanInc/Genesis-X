// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef SQLCONFIG_H
#define SQLCONFIG_H

#include <GenesisX/Orm/genesisx_orm_global.h>
#include <QVariantMap>
#include <QString>

namespace gx::orm {

struct GENESISX_ORM_EXPORT SqlConfig {
    QString driver;
    QString host;
    int port = 0;
    QString database;
    QString user;
    QString password;
    QVariantMap options;
};

}

#endif // SQLCONFIG_H
