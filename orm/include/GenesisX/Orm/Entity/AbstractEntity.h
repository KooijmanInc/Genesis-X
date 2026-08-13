// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef ABSTRACTENTITY_H
#define ABSTRACTENTITY_H

#include <QObject>
#include <QScopedPointer>

#include <GenesisX/Orm/Core/genesisx_orm_global.h>

// #include <GenesisX/Orm/Repository/AbstractRepository.h>

#include <GenesisX/Orm/Data/StringDecorator.h>

namespace gx::orm {

class AbstractRepository;

class GENESISX_ORM_EXPORT AbstractEntity : public QObject
{
    Q_OBJECT

    friend class AbstractRepository;

public:
    AbstractEntity(QObject* parent = nullptr, const QString& key = "EntityKey");
    AbstractEntity(QObject* parent, const QString& key, const QJsonObject& jsonObject);
    virtual ~AbstractEntity() override;

public:
    const QString& id() const;
    const QString& key() const;
    void update(const QJsonObject& jsonObject);

signals:
    void dataDecoratorsChanged();

private:
    void setRepository(AbstractRepository* repository);

protected:
    DataDecorator* addDataItem(DataDecorator* dataDecorator);

protected:
    class Implementation;
    QScopedPointer<Implementation> implementation;
};

}

#endif // ABSTRACTENTITY_H
