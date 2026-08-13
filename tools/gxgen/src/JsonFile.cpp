// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "JsonFile.h"

#include <QFile>

JsonFile::JsonFile() {}

QJsonObject JsonFile::loadJsonFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return {};
    const auto doc = QJsonDocument::fromJson(f.readAll());
    return doc.isObject() ? doc.object() : QJsonObject{};
}

bool JsonFile::saveJsonFile(const QString &path, const QJsonObject &j)
{
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(j).toJson(QJsonDocument::Indented));
        f.close();

        return true;
    }

    return false;
}
