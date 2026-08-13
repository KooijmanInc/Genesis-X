// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef STRINGDECORATOR_H
#define STRINGDECORATOR_H

#include <QObject>
#include <QScopedPointer>

#include <GenesisX/Orm/Core/genesisx_orm_global.h>
#include <GenesisX/Orm/Data/DataDecorator.h>

namespace gx::orm {

class GENESISX_ORM_EXPORT StringDecorator : public DataDecorator
{
    Q_OBJECT

    Q_PROPERTY(QString value READ value WRITE setValue NOTIFY valueChanged)

public:
    StringDecorator(AbstractEntity* parentEntity = nullptr, const QString& key = "itemKey", const QString& label = "", const QString& value = "");
    virtual ~StringDecorator() override;

    StringDecorator& setValue(const QString& value);
    const QString& value() const;

    QJsonValue jsonValue() const override;
    void update(const QJsonObject& jsonObject) override;

signals:
    void valueChanged();

private:
    class Implementation;
    QScopedPointer<Implementation> implementation;
};

}

#endif // STRINGDECORATOR_H
