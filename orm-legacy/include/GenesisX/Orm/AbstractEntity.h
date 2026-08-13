// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef ABSTRACTENTITY_H
#define ABSTRACTENTITY_H

#include <QObject>
#include <QUrl>

#include <GenesisX/Orm/genesisx_orm_global.h>

namespace gx::orm::entity {

class GENESISX_ORM_EXPORT AbstractEntity : public QObject
{
    Q_OBJECT

public:
    explicit AbstractEntity(QObject* parent = nullptr);
    ~AbstractEntity() override = default;

protected:
    static QString fileToBase64(const QUrl& fileUrl, QString* outError = nullptr);

    static QString fileToBase64Path(const QString& filePath, QString* outError = nullptr);
};

}

#endif // ABSTRACTENTITY_H
