// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef JSON_H
#define JSON_H

#include <GenesisX/Orm/Core/genesisx_orm_global.h>

namespace gx::orm {

class GENESISX_ORM_EXPORT Json
{
public:
    QByteArray getByteArrayFromJsonObject(const QJsonObject& object);
    QJsonObject getJsonObjectFromByteArray(const QByteArray& byteArray);
};

}

#endif // JSON_H
