// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef JSONFILE_H
#define JSONFILE_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

class JsonFile
{
public:
    JsonFile();

    QJsonObject loadJsonFile(const QString& path);

    bool saveJsonFile(const QString& path, const QJsonObject& j);
};

#endif // JSONFILE_H
