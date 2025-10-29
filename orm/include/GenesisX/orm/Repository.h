// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <GenesisX/Orm/genesisx_orm_global.h>

#include <QVector>
#include <QObject>
#include <QVariantMap>
#include <QVariantList>
#include <QtQml/QQmlListProperty>

namespace gx::orm {

class GENESISX_ORM_EXPORT Repository : public QObject
{
    Q_OBJECT

public:
    explicit Repository(QObject* parent = nullptr, QString name = {})
        : QObject{parent}, m_name(std::move(name)) {}

    class FetchResult
    {
    public:
        explicit FetchResult(QVector<QObject*> rows, QObject* owner)
            : m_rows(std::move(rows)), m_owner(owner) {}

        QQmlListProperty<QObject> asQmlList() {
            return QQmlListProperty<QObject>(
                m_owner, this, &FetchResult::count, &FetchResult::at
            );
        }

        QVariantList asVariantList() const {
            QVariantList out;
            out.reserve(m_rows.size());
            for (auto* o : m_rows) out.push_back(QVariant::fromValue(o));
            return out;
        }

        QObject* one() const { return m_rows.isEmpty() ? nullptr : m_rows.first(); }

        const QVector<QObject*>& rows() const { return m_rows; }

    private:
        static qsizetype count(QQmlListProperty<QObject>* p) {
            return static_cast<FetchResult*>(p->data)->m_rows.size();
        }
        static QObject* at(QQmlListProperty<QObject>* p, qsizetype i) {
            auto* self = static_cast<FetchResult*>(p->data);
            return (i >= 0 && i < self->m_rows.size()) ? self->m_rows[i] : nullptr;
        }

        QVector<QObject*> m_rows;
        QObject* m_owner{};
    };

    FetchResult fetch(const QVariantMap& conditions = {}) {
        return FetchResult(fetchImpl(conditions), this);
    }

protected:
    virtual QVector<QObject*> fetchImpl(const QVariantMap& conditions) = 0;
    QString m_name;
};

}

#endif // REPOSITORY_H
