// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef SQLSCHEMA_H
#define SQLSCHEMA_H

#include <GenesisX/Orm/Core/genesisx_orm_global.h>

#include <QSqlDatabase>

namespace gx::orm {

enum class ColumnType
{
    TinyInteger,
    SmallInteger,
    MediumInteger,
    Integer,
    Int,
    BigInteger,
    Float,
    Double,
    Decimal,
    Date,
    TimeStamp,
    Time,
    Year,
    Char,
    VarChar,
    TinyBlob,
    Blob,
    MediumBlob,
    LongBlob,
    TinyText,
    Text,
    MediumText,
    LongText,
    Enum,
    Set
};

struct ColumnDefinition
{
    QString name;
    QString type;
    QString comment = {};
    bool nullable = true;
    bool primaryKey = false;
    bool autoIncrement = false;
    bool unique = false;
};

struct TableDefinition
{
    QString name;
    std::vector<ColumnDefinition> columns;
    bool ifNotExists = true;
};

class SqlConnection;

class GENESISX_ORM_EXPORT SqlSchema final
{
public:
    explicit SqlSchema();

    void setSqlConnection(QSqlDatabase sqlConnection);

    QString createTable(const TableDefinition& table);

    QSqlDatabase sqlConnection() const;

    QString columnTypeToString(ColumnType type);

private:
    QString quoteIdentifier(const QString& identifier) const;

    QSqlDatabase m_sqlConnection;
};

}

#endif // SQLSCHEMA_H
