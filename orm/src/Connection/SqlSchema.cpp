// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/Connection/SqlSchema.h>
#include <GenesisX/Orm/Connection/SqlConnection.h>

#include <QSqlQuery>
#include <QSqlError>

namespace gx::orm {

SqlSchema::SqlSchema()
{

}

void SqlSchema::setSqlConnection(QSqlDatabase sqlConnection)
{
    m_sqlConnection = sqlConnection;
}

QString SqlSchema::createTable(const TableDefinition &table)
{
    if (table.name.isEmpty() || table.columns.empty()) {
        return {};
    }

    QStringList columns;

    for (const auto& column : table.columns) {
        QString definition = quoteIdentifier(column.name)
        + " "
        + column.type;

        if (column.primaryKey) {
            definition += " PRIMARY KEY";
        }

        if (column.autoIncrement) {
            definition += " AUTOINCREMENT";
        }

        if (!column.nullable) {
            definition += " NOT NULL";
        }

        if (column.unique) {
            definition += " UNIQUE";
        }

        columns.append(definition);
    }

    QString query = "CREATE TABLE ";
    if (table.ifNotExists) {
        query += "IF NOT EXISTS ";
    }

    query += quoteIdentifier(table.name);
    query += " (";
    query += columns.join(", ");
    query += ")";

    QSqlQuery sqlQuery{m_sqlConnection};

    // if (!sqlQuery.exec(query)) {
    //     qCritical()
    //         << "[GX ORM] Unable to create table:"
    //         << sqlQuery.lastError().text();
    //     return false;
    // }

    return query;
}

QString SqlSchema::quoteIdentifier(const QString &identifier) const
{
    QString value = identifier;
    value.replace("\"", "\"\"");

    return "\"" + value + "\"";
}

QString SqlSchema::columnTypeToString(ColumnType type)
{
    switch (type) {
    case ColumnType::TinyInteger:
        return "TINYINT";
    case ColumnType::SmallInteger:
        return "SMALLINT";
    case ColumnType::MediumInteger:
        return "MEDIUMINT";
    case ColumnType::Int:
        return "INT";
    case ColumnType::Integer:
        return "INTEGER";
    case ColumnType::BigInteger:
        return "BIGINT";
    case ColumnType::Float:
        return "FLOAT";
    case ColumnType::Double:
        return "DOUBLE";
    case ColumnType::Decimal:
        return "DECIMAL";
    case ColumnType::Date:
        return "DATE";
    case ColumnType::TimeStamp:
        return "TIMESTAMP";
    case ColumnType::Time:
        return "TIME";
    case ColumnType::Year:
        return "YEAR";
    case ColumnType::Char:
        return "CHAR";
    case ColumnType::VarChar:
        return "VARCHAR";
    case ColumnType::TinyBlob:
        return "TINYBLOB";
    case ColumnType::Blob:
        return "BLOB";
    case ColumnType::MediumBlob:
        return "MEDIUMBLOB";
    case ColumnType::LongBlob:
        return "LONGBLOB";
    case ColumnType::TinyText:
        return "TINYTEXT";
    case ColumnType::Text:
        return "TEXT";
    case ColumnType::MediumText:
        return "MEDIUNTEXT";
    case ColumnType::LongText:
        return "LONGTEXT";
    case ColumnType::Enum:
        return "ENUM";
    case ColumnType::Set:
        return "SET";
    }

    return {};
}

QSqlDatabase SqlSchema::sqlConnection() const
{
    return m_sqlConnection;
}

}