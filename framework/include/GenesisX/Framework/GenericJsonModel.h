// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 The Kooijman Incorporate Holding B.V.

#ifndef GENERICJSONMODEL_H
#define GENERICJSONMODEL_H

#include <GenesisX/Framework/genesisx_framework_global.h>

#include <QAbstractListModel>
#include <QJsonArray>
#include <QVariantMap>

namespace gx::framework {

class GENESISX_FRAMEWORK_EXPORT GenericJsonModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY (int count READ rowCount NOTIFY countChanged)

public:
    explicit GenericJsonModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void loadFromJson(const QJsonArray &array);

    Q_INVOKABLE QVariantMap row(int index) const;

signals:
    void countChanged();

private:
    QList<QVariantMap> m_rows;
    QList<QByteArray> m_roleNames;
};

}

#endif // GENERICJSONMODEL_H
