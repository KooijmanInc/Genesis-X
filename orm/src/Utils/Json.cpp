// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/Utils/Json.h>

#include <QJsonDocument>
#include <QJsonObject>

namespace gx::orm {

QByteArray Json::getByteArrayFromJsonObject(const QJsonObject &object)
{
    const QByteArray bytes = QJsonDocument(object).toJson(QJsonDocument::Compact);

    return bytes;
}

QJsonObject Json::getJsonObjectFromByteArray(const QByteArray &byteArray)
{
    QJsonParseError error;

    const QJsonDocument document = QJsonDocument::fromJson(byteArray, &error);

    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        qCritical() << "Invalid JSON:" << error.errorString();
        return {};
    }

    const QJsonObject object = document.object();

    return object;
}

}