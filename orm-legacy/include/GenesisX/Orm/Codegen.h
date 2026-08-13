// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef CODEGEN_H
#define CODEGEN_H

#include <QString>
#include <QStringList>
#include <QSqlDatabase>

#include <GenesisX/Orm/genesisx_orm_global.h>

namespace gx::orm::codegen {

/**
 * @brief mapSqlToCpp   Map an engine type to a C++ type string (QString, qint32, QDateTime, ...).
 *                      All hints are used to produce the best mapping.
 *
 * @param driver        Qt driver name (qmysql, qpsql, qsqlite, qodbc, qoci)
 * @param typeName      Raw DB type string (e.g., "varchar(255)", "enum('0','1')", "timestamp with time sone")
 * @param length        Length / precision (total digits) if available; may be 0
 * @param precision     Numeric scale if available; may be 0
 * @param isUnsigned    true if the column is unsigned (mainly MySql)
 * @param isBoolHint    true if caller inferred boolean semantics from context
 * @return
 */
GENESISX_ORM_EXPORT
QString mapSqlToCpp(const QString& driver, const QString& typeName, int length, int scale, bool isUnsignedIn, bool isBoolHint);

/**
 * @brief generateModel Generate Entity/Repository/DataAccess headers into outDir using an OPEN DB.
 *
 * @param db            Open QSqlDatabase
 * @param databaseName  Database (catelog) name (MySQL/MSSQL); for PG you can also pass schema here if schemaName empty
 * @param schemaName    Optional schema name (PG/MSSQL/Oracle). Leave empty to use default (e.g., current_schema()/dbo)
 * @param tableName     Table to introspect
 * @param outDir        Output base dir; generator will create entities/, repositories/, dataaccess/
 * @param ns            C++ namespace for generated code (default "App")
 * @return              Empty string on success, or an error message.
 */
GENESISX_ORM_EXPORT
QString generateModel(const QSqlDatabase& db, const QString& databaseName, const QString& schemaName, const QString& tableName, const QString& outDir, const QString& ns = "App");

/**
 * Same as generateModel(), but creates a temporary connection internally.
 *
 * For SQLite, pass driverName="QSQLITE", host as the database file path,
 * databaseName may be empty, schemaName ignored
 */
GENESISX_ORM_EXPORT
QString generateModelWithConn(const QString& driverName, const QString& host, int port, const QString& user, const QString& pass, const QString& databaseName, const QString& schemaName, const QString& tableName, const QString& outDir, const QString& ns = "App", const QString& flavorOverride = QString());

}

#endif // CODEGEN_H
