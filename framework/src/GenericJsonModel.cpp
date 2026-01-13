// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 The Kooijman Incorporate Holding B.V.

#include <GenesisX/Framework/GenericJsonModel.h>
#include <QJsonObject>

using namespace gx::framework;


GenericJsonModel::GenericJsonModel(QObject *parent)
    : QAbstractListModel{parent}
{
    qDebug() << "initiated";
}

int GenericJsonModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;

    return m_rows.count();
}

QVariant GenericJsonModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size()) return {};

    const QVariantMap &row = m_rows.at(index.row());
    const QByteArray roleName = roleNames().value(role);

    return row.value(QString::fromUtf8(roleName));
}

QHash<int, QByteArray> GenericJsonModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    for (int i = 0; i < m_roleNames.size(); ++i) roles[Qt::UserRole + 1 + i] = m_roleNames.at(i);

    return roles;
}

void GenericJsonModel::loadFromJson(const QJsonArray &array)
{
    qDebug() << array;
    beginResetModel();

    m_rows.clear();
    m_roleNames.clear();

    if (!array.isEmpty()) {
        QJsonObject first = array.first().toObject();
        for (const QString& key : first.keys()) m_roleNames.append(key.toUtf8());
    }

    for (const auto& v : array) {
        QJsonObject obj = v.toObject();
        QVariantMap map;

        for (const QByteArray &role : m_roleNames) map.insert(QString::fromUtf8(role), obj.value(QString::fromUtf8(role)).toVariant());

        m_rows.append(map);
    }

    endResetModel();
    emit countChanged();
}

QVariantMap GenericJsonModel::row(int index) const
{
    if (index < 0 || index >= m_rows.size()) return {};

    return m_rows.at(index);
}

