// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef DATADECORATOR_H
#define DATADECORATOR_H

#include <GenesisX/Orm/Core/genesisx_orm_global.h>

#include <QObject>
#include <QJsonObject>
#include <QScopedPointer>

namespace gx::orm {

class AbstractEntity;

class GENESISX_ORM_EXPORT DataDecorator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString label READ label CONSTANT)

public:
    DataDecorator(AbstractEntity *parent = nullptr, const QString& key = "itemKey", const QString& label = "");
    virtual ~DataDecorator() override;

    const QString& key() const;
    const QString& label() const;

    virtual QJsonValue jsonValue() const = 0;
    virtual void update(const QJsonObject& jsonObject) = 0;

private:
    class Implementation;
    QScopedPointer<Implementation> implementation;
};

}

#endif // DATADECORATOR_H
