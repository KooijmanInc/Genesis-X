// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/Entity/AbstractEntity.h>
#include <GenesisX/Orm/Repository/AbstractRepository.h>

#include <map>

#include <QJsonArray>

namespace gx::orm {

class AbstractEntity::Implementation
{
public:
    Implementation(AbstractEntity* _entity, const QString& _key)
        : abstractEntity{_entity}
        , key{_key}
        , id(QUuid::createUuid().toString())
    {}

    AbstractEntity* abstractEntity{nullptr};
    AbstractRepository* repository{nullptr};

    QString key;
    QString id;
    GXOrm::StringDecorator* primaryKey{nullptr};

    std::map<QString, DataDecorator*> dataDecorators;
};

AbstractEntity::AbstractEntity(QObject *parent, const QString &key)
    : QObject{parent}
{
    implementation.reset(new Implementation(this, key));
}

AbstractEntity::AbstractEntity(QObject *parent, const QString &key, const QJsonObject &jsonObject)
    : AbstractEntity{parent, key}
{
    update(jsonObject);
}

AbstractEntity::~AbstractEntity()
{}

const QString &AbstractEntity::id() const
{
    if (implementation->primaryKey != nullptr && !implementation->primaryKey->value().isEmpty()) {
        return implementation->primaryKey->value();
    }

    return implementation->id;
}

const QString &AbstractEntity::key() const
{
    return implementation->key;
}

void AbstractEntity::update(const QJsonObject &jsonObject)
{
    if (jsonObject.contains("id")) {
        implementation->id = jsonObject.value("id").toString();
    }
}

void AbstractEntity::setRepository(AbstractRepository *repository)
{
    implementation->repository = repository;
}

DataDecorator *AbstractEntity::addDataItem(DataDecorator *dataDecorator)
{
    if (implementation->dataDecorators.find(dataDecorator->key()) == std::end(implementation->dataDecorators)) {
        implementation->dataDecorators[dataDecorator->key()] = dataDecorator;

        emit dataDecoratorsChanged();
    }

    return dataDecorator;
}



}