// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/AbstractEntity.h>

#include <QFile>

using namespace gx::orm::entity;


AbstractEntity::AbstractEntity(QObject *parent)
    : QObject{parent}
{
}

QString AbstractEntity::fileToBase64(const QUrl &fileUrl, QString *outError)
{
    const QString path = fileUrl.isLocalFile() ? fileUrl.toLocalFile() : fileUrl.toString();
    return fileToBase64Path(path, outError);
}

QString AbstractEntity::fileToBase64Path(const QString &filePath, QString *outError)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        if (outError) *outError = QStringLiteral("Failed to open file: %1").arg(filePath);
        return {};
    }

    const QByteArray bytes = f.readAll();
    if (bytes.isEmpty()) {
        if (outError) *outError = QStringLiteral("File is empty: %1").arg(filePath);
        return {};
    }

    return QString::fromLatin1(bytes.toBase64());
}
