// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/Data/DataDecorator.h>
#include <GenesisX/Orm/Entity/AbstractEntity.h>

namespace gx::orm {

class DataDecorator::Implementation
{
public:
    Implementation(AbstractEntity* _parent, const QString& _key, const QString& _label)
        : parentEntity{_parent}
        , key{_key}
        , label{_label}
    {}

    AbstractEntity* parentEntity{nullptr};
    QString key;
    QString label;
};

DataDecorator::DataDecorator(AbstractEntity *parent, const QString &key, const QString &label)
    : QObject{(QObject*)parent}
{
    implementation.reset(new Implementation(parent, key, label));
}

DataDecorator::~DataDecorator()
{}

const QString &DataDecorator::key() const
{
    return implementation->key;
}

const QString &DataDecorator::label() const
{
    return implementation->label;
}

}