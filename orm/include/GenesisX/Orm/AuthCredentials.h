// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef AUTHCREDENTIALS_H
#define AUTHCREDENTIALS_H

#include "genesisx_orm_global.h"
#include <QString>

namespace gx::orm {

struct GENESISX_ORM_EXPORT AuthCredentials
{
    QString appKey;
    QString apiToken;
    QString bearerToken;
    bool hasAuth() const { return !appKey.isEmpty() || !apiToken.isEmpty() || !bearerToken.isEmpty(); }
};

}

#endif // AUTHCREDENTIALS_H
