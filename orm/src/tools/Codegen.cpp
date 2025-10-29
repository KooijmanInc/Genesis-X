// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/Codegen.h>
#include <QRegularExpression>
#include <QStringConverter>
#include <QSqlDatabase>
#include <QTextStream>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlField>
#include <QSaveFile>
#include <QFileInfo>
#include <QSettings>
#include <QVariant>
#include <QUuid>
#include <QDir>

/*!
 *  \headerfile Codegen.h
 *  \inmodule GenesisX/Orm
 *  \ingroup genesisx-orm
 *  \since 6.10
 */
namespace gx::orm::codegen {

struct GxCodegenConfig {
    enum class JsonMode {
        AsQString,
        AsQJsonDocument
    };
    JsonMode jsonMode = JsonMode::AsQString;
    int decimalIntegerThreshold = 18;
};
static const GxCodegenConfig kCfg{};

enum class DbFlavor { MySQL, PostgreSQL, SQLite, MSSQL, Oracle, Unknown };

static QString detectOdbcMysqlDriver()
{
#ifdef Q_OS_WIN
    // 64-bit ODBC drivers
    QSettings s(R"(HKEY_LOCAL_MACHINE\SOFTWARE\ODBC\ODBCINST.INI\ODBC Drivers)", QSettings::NativeFormat);
    const QStringList keys = s.allKeys();  // driver display names

    auto has = [&](const QString& needle) {
        for (const QString& k : keys) if (k.contains(needle, Qt::CaseInsensitive)) return true;
        return false;
    };
    // Prefer newest MySQL Unicode, then MariaDB
    QStringList prefs = {
        "MySQL ODBC 9.5 Unicode Driver",
        "MySQL ODBC 9.0 Unicode Driver",
        "MySQL ODBC 8.0 Unicode Driver",
        "MariaDB ODBC 3.2 Driver",
        "MariaDB ODBC 3.1 Driver"
    };
    for (const QString& p : prefs) if (has(p)) return p;
#endif
    // Cross-platform or unknown: let ODBC try these in order
    return QStringLiteral("MySQL ODBC 9.5 Unicode Driver");
}

// ---- helpers for probing ------------------------------------------------
static QString execScalarString(QSqlDatabase db, const QString& sql) {
    QSqlQuery q(db);
    if (!q.exec(sql)) return {};
    if (!q.next()) return {};
    return q.value(0).toString();
}

static bool execOk(QSqlDatabase db, const QString& sql) {
    QSqlQuery q(db);
    return q.exec(sql);
}

// Normalize user override strings like "mysql", "mssql", "postgresql", "sqlite", "oracle"
static std::optional<DbFlavor> parseFlavorOverride(const QString& s) {
    const QString v = s.trimmed().toLower();

    if (v.isEmpty()) return std::nullopt;
    if (v == "mysql" || v == "mariadb")      return DbFlavor::MySQL;
    if (v == "postgres" || v == "postgresql")return DbFlavor::PostgreSQL;
    if (v == "sqlite")                       return DbFlavor::SQLite;
    if (v == "mssql" || v == "sqlserver")    return DbFlavor::MSSQL;
    if (v == "oracle")                       return DbFlavor::Oracle;
    return std::nullopt;
}

// Try to identify DBMS by asking the server.
static DbFlavor probeFlavor(QSqlDatabase db) {
    // ---- MySQL / MariaDB strong signals (won't exist on SQL Server/Postgres/Oracle) ----
    {
        // 1) @@global.version exists only on MySQL/MariaDB
        const QString ver = execScalarString(db, QStringLiteral("SELECT @@global.version"));
        if (!ver.isEmpty()) return DbFlavor::MySQL;
    }
    {
        // 2) @@version_comment is MySQL/MariaDB specific (often "MySQL Community Server" or "MariaDB")
        const QString vc = execScalarString(db, QStringLiteral("SELECT @@version_comment"));
        if (!vc.isEmpty()) return DbFlavor::MySQL;
    }
    {
        // 3) SHOW VARIABLES works on MySQL/MariaDB; use it as a boolean probe
        if (execOk(db, QStringLiteral("SHOW VARIABLES LIKE 'version'")))
            return DbFlavor::MySQL;
    }
    {
        // 4) VERSION() — some ODBC stacks return numeric only; accept numeric as MySQL-ish
        const QString v = execScalarString(db, QStringLiteral("SELECT VERSION()"));
        if (!v.isEmpty()) {
            const QString l = v.toLower();
            if (l.contains("mysql") || l.contains("mariadb")) return DbFlavor::MySQL;
            // Pure numeric (e.g. "8.0.43-ubuntu…") — PostgreSQL's version() never returns just numbers
            static const QRegularExpression re(R"(^\d+\.\d+(\.\d+)?([-+].*)?$)");
            if (re.match(v).hasMatch()) return DbFlavor::MySQL;
        }
    }

    // ---- PostgreSQL ----
    {
        const QString v = execScalarString(db, QStringLiteral("SELECT version()"));
        if (!v.isEmpty() && v.toLower().contains("postgresql")) return DbFlavor::PostgreSQL;
    }

    // ---- SQL Server (ODBC) ----
    {
        const QString v1 = execScalarString(db, QStringLiteral("SELECT CAST(SERVERPROPERTY('ProductVersion') AS NVARCHAR(128))"));
        if (!v1.isEmpty()) return DbFlavor::MSSQL;

        const QString v2 = execScalarString(db, QStringLiteral("SELECT @@VERSION"));
        if (!v2.isEmpty() && v2.toLower().contains("microsoft sql server")) return DbFlavor::MSSQL;
    }

    // ---- Oracle ----
    {
        if (execOk(db, QStringLiteral("SELECT * FROM v$version"))) return DbFlavor::Oracle;
        const QString v = execScalarString(db, QStringLiteral("SELECT banner FROM v$version"));
        if (!v.isEmpty() && v.toLower().contains("oracle")) return DbFlavor::Oracle;
        const QString who = execScalarString(db, QStringLiteral("SELECT user FROM dual"));
        if (!who.isEmpty()) return DbFlavor::Oracle;
    }

    // ---- SQLite ----
    {
        const QString v = execScalarString(db, QStringLiteral("SELECT sqlite_version()"));
        if (!v.isEmpty()) return DbFlavor::SQLite;
    }

    return DbFlavor::Unknown;
}

// Keep a fast path by driver, but fall back to probing for QODBC.
static DbFlavor detectFlavorSmart(QSqlDatabase db) {
    const QString d = db.driverName().toLower();

    if (d.contains("qmysql"))  return DbFlavor::MySQL;
    if (d.contains("qpsql"))   return DbFlavor::PostgreSQL;
    if (d.contains("qsqlite")) return DbFlavor::SQLite;
    if (d.contains("qoci"))    return DbFlavor::Oracle;

    // QODBC or unknown drivers → probe
    return probeFlavor(db);
}

static QString qIdent(DbFlavor f, const QString& name)
{
    if (name.isEmpty()) return name;
    switch (f) {
    case DbFlavor::MySQL: return QString("`%1`").arg(QString(name).replace('`', "``"));
    case DbFlavor::PostgreSQL: return QString("\"%1\"").arg(QString(name).replace('"', "\"\""));
    case DbFlavor::SQLite: return QString("\"%1\"").arg(QString(name).replace('"', "\"\""));
    case DbFlavor::MSSQL: return QString("[%1]").arg(QString(name).replace(']', "]]"));
    case DbFlavor::Oracle: return QString("\"%1\"").arg(QString(name).replace('"', "\"\""));
    default: return name;
    }
}

static QString qQualified(DbFlavor f, const QString& database, const QString& schema, const QString& table)
{
    const QString qt = qIdent(f, table);
    switch (f) {
    case DbFlavor::MySQL:
        return database.isEmpty() ? qt : qIdent(f, database) + "." + qt;
    case DbFlavor::PostgreSQL: {
        const QString sch = schema.isEmpty() ? database : schema;
        return qIdent(f,sch) + "." + qt;
    }
    case DbFlavor::MSSQL: {
        const QString sch = schema.isEmpty() ? "dbo" : schema;
        return qIdent(f, sch) + "." + qt;
    }
    case DbFlavor::SQLite: return qt;
    case DbFlavor::Oracle: {
        const QString sch = schema.isEmpty() ? database : schema;
        return qIdent(f, sch) + "." + qt;
    }
    default:
        return qt;
    }
}

static void setDbContext(QSqlDatabase db, DbFlavor f, const QString& database, const QString& schema)
{
    QSqlQuery q(db);
    switch (f) {
    case DbFlavor::MySQL:
        if (!database.isEmpty()) q.exec("USE " + qIdent(f, database));
        break;
    case DbFlavor::PostgreSQL:
        if (!schema.isEmpty())
            q.exec("SET search_path TO " + qIdent(f, schema));
        else if (!database.isEmpty())
            q.exec("SET search_path TO " + qIdent(f, database));
        break;
    case DbFlavor::MSSQL:
        if (!database.isEmpty()) q.exec("USE " + qIdent(f, database));
        break;
    case DbFlavor::Oracle:
        if (!schema.isEmpty())
            q.exec("ALTER SESSION SET CURRENT_SCHEMA = " + qIdent(f, schema));
        break;
    case DbFlavor::SQLite:
    default:
        break;
    }
}

struct ColMeta {
    QString name;
    QString dataType;
    bool isUnsigned = false;
    int length = 0;
    int scale = 0;
    bool isNullable = true;
};

static inline QString baseTypeOf(const QString& tn)
{
    QString t = tn.trimmed().toLower();
    t.replace("unsigned", "").replace("with time zone", "").replace("without time zone", "");
    int paren = t.indexOf('(');
    if (paren >= 0) t = t.left(paren);

    return t.trimmed();
}

static QString pascal(const QString& s)
{
    QStringList parts = s.trimmed().split(QRegularExpression(R"([_\s]+)"), Qt::SkipEmptyParts);

    for (auto& p : parts) { if (!p.isEmpty()) p[0] = p[0].toUpper(); }
    return parts.join("");
}

static QString camel(const QString& s)
{
    QString p = pascal(s);
    if (!p.isEmpty()) p[0] = p[0].toLower();
    return p;
}

static inline bool hasEnum01(const QString& tn)
{
    static const QRegularExpression reBoolEnum(
        R"(^enum\s*\(\s*('?0'?)\s*,\s*('?1'?)\s*\)|^enum\s*\(\s*('?1'?)\s*,\s*('?0'?)\s*\)))",
        QRegularExpression::CaseInsensitiveOption);

    return reBoolEnum.match(tn).hasMatch();
}

static inline void extractParams(const QString& tn, int& len, int& scale)
{
    int l = tn.indexOf('('), r = tn.indexOf(')');
    if (l >= 0 && r > l) {
        const QString inside = tn.mid(l+1, r-l-1);
        const QStringList parts = inside.split(',', Qt::SkipEmptyParts);
        if (parts.size() >= 1) len = parts[0].trimmed().toInt();
        if (parts.size() >= 2) scale = parts[1].trimmed().toInt();
    }
}

static QList<ColMeta> fetchColumns(QSqlDatabase db, DbFlavor f, const QString database, const QString schema, const QString table)
{
    QList<ColMeta> cols;
    QSqlQuery q(db);

    switch (f) {
    // case DbFlavor::MySQL: {
    //     const bool hasDb = !database.isEmpty();
    //     q.prepare(R"SQL(
    //         SELECT COLUMN_NAME, DATA_TYPE, COLUMN_TYPE, IS_NULLABLE,
    //                COALESCE(CHARACTER_MAXIMUM_LENGTH, NUMERIC_PRECISION) AS LEN,
    //                COALESCE(NUMERIC_SCALE, 0) AS SCALE
    //         FROM INFORMATION_SCHEMA.COLUMNS
    //         WHERE TABLE_SCHEMA = COALESCE(NULLIF(?,''), DATABASE())
    //           AND TABLE_NAME = ?
    //         ORDER BY ORDINAL_POSITION
    //     )SQL");
    //     // const QString sch = schema.isEmpty() ? database : schema;
    //     // q.addBindValue(sch);
    //     // q.addBindValue(table);
    //     // if (!q.exec()) return cols;
    //     // while (q.next()) {
    //     //     ColMeta m;
    //     //     m.name = q.value(0).toString();
    //     //     m.dataType = q.value(1).toString().toLower();
    //     //     const QString columnType = q.value(2).toString().toLower();
    //     //     m.isUnsigned = columnType.contains("unsigned");
    //     //     m.isNullable = q.value(3).toString().compare("yes", Qt::CaseInsensitive) == 0;
    //     //     m.length = q.value(4).toInt();
    //     //     m.scale = q.value(5).toInt();
    //     //     cols.push_back(m);
    //     // }
    //     // In MySQL, TABLE_SCHEMA must be the **database** (catalog) name
    //     // const QString sch = database;  // ignore `schema` for MySQL
    //     q.addBindValue(hasDb ? database : QString{});
    //     q.addBindValue(table);

    //     bool ok = q.exec();
    //     bool any = false;
    //     while (ok && q.next()) {
    //         any = true;
    //         ColMeta m;
    //         m.name       = q.value(0).toString();
    //         m.dataType   = q.value(1).toString().toLower();
    //         const QString columnType = q.value(2).toString().toLower();
    //         m.isUnsigned = columnType.contains("unsigned");
    //         m.isNullable = q.value(3).toString().compare("yes", Qt::CaseInsensitive) == 0;
    //         m.length     = q.value(4).toInt();
    //         m.scale      = q.value(5).toInt();
    //         cols.push_back(m);
    //     }

    //     if (!any) {
    //         // Fallback that works even with limited permissions
    //         cols.clear();
    //         QSqlQuery show(db);
    //         const QString sql = QStringLiteral("SHOW COLUMNS FROM %1.%2")
    //                                 .arg(qIdent(f, database), qIdent(f, table));
    //         if (!show.exec(sql)) return cols;

    //         // SHOW returns: Field, Type, Null, Key, Default, Extra
    //         while (show.next()) {
    //             ColMeta m;
    //             m.name = show.value(0).toString();
    //             const QString type = show.value(1).toString().toLower(); // e.g. "int(11) unsigned"
    //             m.dataType = baseTypeOf(type);
    //             m.isUnsigned = type.contains("unsigned");
    //             m.isNullable = (show.value(2).toString().compare("YES", Qt::CaseInsensitive) == 0);
    //             extractParams(type, m.length, m.scale);
    //             cols.push_back(m);
    //         }
    //     }
    //     break;
    // }
    case DbFlavor::MySQL: {
        // 1) Try INFORMATION_SCHEMA with a safe default (DATABASE()) when schema empty
        const bool hasDb = !database.isEmpty();
        q.prepare(R"SQL(
        SELECT COLUMN_NAME, DATA_TYPE, COLUMN_TYPE, IS_NULLABLE,
               COALESCE(CHARACTER_MAXIMUM_LENGTH, NUMERIC_PRECISION) AS LEN,
               COALESCE(NUMERIC_SCALE, 0) AS SCALE
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_SCHEMA = COALESCE(NULLIF(?,''), DATABASE())
          AND TABLE_NAME = ?
        ORDER BY ORDINAL_POSITION
    )SQL");
        q.addBindValue(hasDb ? database : QString{});
        q.addBindValue(table);

        bool ok = q.exec();
        bool any = false;
        while (ok && q.next()) {
            any = true;
            ColMeta m;
            m.name       = q.value(0).toString();
            m.dataType   = q.value(1).toString().toLower();
            const QString columnType = q.value(2).toString().toLower();
            m.isUnsigned = columnType.contains("unsigned");
            m.isNullable = q.value(3).toString().compare("yes", Qt::CaseInsensitive) == 0;
            m.length     = q.value(4).toInt();
            m.scale      = q.value(5).toInt();
            cols.push_back(m);
        }

        if (!any) {
            // 2) Fallback: SHOW COLUMNS
            cols.clear();
            QSqlQuery show(db);
            QString sql;
            if (hasDb) {
                sql = QStringLiteral("SHOW COLUMNS FROM %1.%2")
                .arg(qIdent(f, database), qIdent(f, table));
            } else {
                sql = QStringLiteral("SHOW COLUMNS FROM %1")
                .arg(qIdent(f, table));
            }
            if (show.exec(sql)) {
                while (show.next()) {
                    ColMeta m;
                    m.name       = show.value(0).toString();                 // Field
                    const QString type = show.value(1).toString().toLower(); // Type (e.g. int(11) unsigned)
                    m.dataType   = baseTypeOf(type);
                    m.isUnsigned = type.contains("unsigned");
                    m.isNullable = (show.value(2).toString().compare("YES", Qt::CaseInsensitive) == 0);
                    extractParams(type, m.length, m.scale);
                    cols.push_back(m);
                }
                if (!cols.isEmpty()) break;
            }

            // 3) Last resort: DESCRIBE (some gateways map this better)
            cols.clear();
            QSqlQuery desc(db);
            QString dsql;
            if (hasDb) {
                dsql = QStringLiteral("DESCRIBE %1.%2").arg(qIdent(f, database), qIdent(f, table));
            } else {
                dsql = QStringLiteral("DESCRIBE %1").arg(qIdent(f, table));
            }
            if (desc.exec(dsql)) {
                while (desc.next()) {
                    ColMeta m;
                    m.name       = desc.value(0).toString();                 // Field
                    const QString type = desc.value(1).toString().toLower(); // Type
                    m.dataType   = baseTypeOf(type);
                    m.isUnsigned = type.contains("unsigned");
                    // Null column might be "YES"/"NO" or bool-ish
                    const auto nullStr = desc.value(2).toString();
                    m.isNullable = nullStr.compare("YES", Qt::CaseInsensitive) == 0 || nullStr == "1";
                    extractParams(type, m.length, m.scale);
                    cols.push_back(m);
                }
            }

            // If still empty, consider privilege issues: we'll let the caller handle it
        }
        break;
    }
    case DbFlavor::PostgreSQL: {
        q.prepare(R"SQL(
            SELECT c.column_name,
                   CASE WHEN t.typcategory = 'E' THEN 'enum' ELSE c.data_type END AS data_type,
                   c.character_maximum_length,
                   c.numeric_precision,
                   c.numeric_scale,
                   c.is_nullable
            FROM information_schema.columns c
            JOIN pg_catalog.pg_class pc ON pc.relname = c.table_name
            JOIN pg_catalog.pg_namespace pn ON pn.nspname = c.table_schema
            JOIN pg_catalog.pg_type t ON t.typname = c.udt_name
            WHERE c.table_schema = COALESCE(NULLIF(?,''), current_schema())
              AND c.table_name = ?
            ORDER BY c.ordinal_position
        )SQL");
        const QString sch = schema.isEmpty() ? database : schema;
        q.addBindValue(sch);
        q.addBindValue(table);
        if (!q.exec()) return cols;
        while (q.next()) {
            ColMeta m;
            m.name       = q.value(0).toString();
            m.dataType   = q.value(1).toString().toLower();
            m.length     = q.value(2).toInt();
            if (m.length == 0) m.length = q.value(3).toInt();
            m.scale      = q.value(4).toInt();
            m.isNullable = q.value(5).toString().compare("yes", Qt::CaseInsensitive) == 0;
            cols.push_back(m);
        }
        break;
    }
    case DbFlavor::MSSQL: {
        q.prepare(R"SQL(
            SELECT c.COLUMN_NAME,
                   c.DATA_TYPE,
                   c.CHARACTER_MAXIMUM_LENGTH,
                   c.NUMERIC_PRECISION,
                   c.NUMERIC_SCALE,
                   c.IS_NULLABLE
            FROM INFORMATION_SCHEMA.COLUMNS c
            WHERE c.TABLE_SCHEMA = COALESCE(?, 'dbo')
              AND c.TABLE_NAME = ?
            ORDER BY c.ORDINAL_POSITION
        )SQL");
        const QString sch = schema.isEmpty() ? "dbo" : schema;
        q.addBindValue(sch);
        q.addBindValue(table);
        if (!q.exec()) return cols;
        while (q.next()) {
            ColMeta m;
            m.name       = q.value(0).toString();
            m.dataType   = q.value(1).toString().toLower();
            m.length     = q.value(2).toInt();
            if (m.length==0) m.length = q.value(3).toInt();
            m.scale      = q.value(4).toInt();
            m.isNullable = q.value(5).toString().compare("yes", Qt::CaseInsensitive) == 0;
            m.isUnsigned = false; // SQL Server doesn't support unsigned ints
            cols.push_back(m);
        }
        break;
    }
    case DbFlavor::SQLite: {
        q.exec("PRAGMA table_info(" + qIdent(f, table) + ")");
        while (q.next()) {
            ColMeta m;
            m.name       = q.value(1).toString();
            m.dataType   = q.value(2).toString().toLower();
            m.isNullable = (q.value(3).toInt() == 0);
            // Try parse (len,scale)
            static QRegularExpression re(R"((\w+)\s*\(\s*(\d+)\s*(?:,\s*(\d+)\s*)?\))");
            auto mt = re.match(m.dataType);
            if (mt.hasMatch()) {
                m.length = mt.captured(2).toInt();
                m.scale  = mt.captured(3).toInt();
                m.dataType = mt.captured(1).toLower();
            }
            cols.push_back(m);
        }
        break;
    }
    case DbFlavor::Oracle: {
        q.prepare(R"SQL(
            SELECT COLUMN_NAME, DATA_TYPE,
                   COALESCE(CHAR_COL_DECL_LENGTH, DATA_LENGTH) AS LEN,
                   DATA_SCALE, NULLABLE
            FROM ALL_TAB_COLUMNS
            WHERE OWNER = COALESCE(?, USER)
              AND TABLE_NAME = ?
            ORDER BY COLUMN_ID
        )SQL");
        const QString sch = schema.isEmpty() ? database.toUpper() : schema.toUpper();
        q.addBindValue(sch);
        q.addBindValue(table.toUpper());
        if (!q.exec()) return cols;
        while (q.next()) {
            ColMeta m;
            m.name       = q.value(0).toString();
            m.dataType   = q.value(1).toString().toLower();
            m.length     = q.value(2).toInt();
            m.scale      = q.value(3).toInt();
            m.isNullable = q.value(4).toString().compare("Y", Qt::CaseInsensitive) == 0;
            cols.push_back(m);
        }
        break;
    }
    default: break;
    }

    return cols;
}

static QString detectPrimaryKey(QSqlDatabase db, DbFlavor f, const QString& database, const QString& schema, const QString& table)
{
    QSqlQuery q(db);
    switch (f) {
    case DbFlavor::MySQL: {
        q.exec("SHOW KEYS FROM " + qIdent(f, table) + " WHERE key_name = 'PRIMARY'");
        if (q.next()) return q.value("Column_name").toString();
        break;
    }
    case DbFlavor::PostgreSQL: {
        q.prepare(R"SQL(
            SELECT a.attname
            FROM pg_index i
            JOIN pg_attribute a ON a.attrelid = i.indrelid AND a.attnum = ANY(i.indkey)
            JOIN pg_class c ON c.oid = i.indrelid
            JOIN pg_namespace n ON n.oid = c.relnamespace
            WHERE i.indisprimary
                AND c.relname = ?
                AND n.nspname = COALESCE(NULLIF(?,''), current_schema())
        )SQL");
        const QString sch = schema.isEmpty() ? database : schema;
        q.addBindValue(table);
        q.addBindValue(sch);
        if (q.exec() && q.next()) return q.value(0).toString();
        break;
    }
    case DbFlavor::MSSQL: {
        q.prepare(R"SQL(
            SELECT c.COLUMN_NAME
            FROM INFORMATION_SCHEMA.TABLE_CONSTRAINTS tc
            JOIN INFORMATION_SCHEMA.KEY_COLUMN_USAGE c
              ON c.CONSTRAINT_NAME = tc.CONSTRAINT_NAME
            WHERE tc.CONSTRAINT_TYPE = 'PRIMARY KEY'
              AND c.TABLE_SCHEMA = COALESCE(?, 'dbo')
              AND c.TABLE_NAME = ?
        )SQL");
        const QString sch = schema.isEmpty() ? "dbo" : schema;
        q.addBindValue(sch);
        q.addBindValue(table);
        if (q.exec() && q.next()) return q.value(0).toString();
        break;
    }
    case DbFlavor::SQLite: {
        q.exec("PRAGMA table_info(" + qIdent(f, table) + ")");
        while (q.next()) if (q.value(5).toInt() == 1) return q.value(1).toString();
        break;
    }
    case DbFlavor::Oracle: {
        q.prepare(R"SQL(
            SELECT cols.column_name
            FROM all_constraints cons
            JOIN all_cons_columns cols
              ON cons.constraint_name = cols.constraint_name
             AND cons.owner = cols.owner
            WHERE cons.constraint_type = 'P'
              AND cons.table_name = ?
              AND cons.owner = COALESCE(?, USER)
        )SQL");
        const QString sch = schema.isEmpty() ? database.toUpper() : schema.toUpper();
        q.addBindValue(table.toUpper());
        q.addBindValue(sch);
        if (q.exec() && q.next()) return q.value(0).toString();
        break;
    }
    default: break;
    }
    return {};
}

QString mapSqlToCpp(const QString& driver, const QString& typeName, int length, int scale, bool isUnsignedIn, bool isBoolHint)
{
    const QString d = driver.trimmed().toLower();
    const QString t = typeName.trimmed().toLower();

    bool isUnsigned = isUnsignedIn || t.contains("unsigned");
    extractParams(t, length, scale);
    const QString base = baseTypeOf(t);

    auto equalsAny = [&](std::initializer_list<const char*> needles) {
        for (const char* n : needles) if (base == QLatin1String(n)) return true; return false;
    };

    // BOOLEAN
    if (isBoolHint) return "bool";
    if (equalsAny({"bool","boolean"})) return "bool";
    if (equalsAny({"bit"}) && (d.contains("qpsql") || d.contains("qodbc"))) return "bool";
    if (base == "tinyint" && (t.startsWith("tinyint(1)") || length == 1)) return "bool";
    if (t.startsWith("enum(") && hasEnum01(t)) return "bool";

    // INTEGERS
    if (base == "tinyint")   return isUnsigned ? "quint8"  : "qint8";
    if (base == "smallint" || base == "int2")   return isUnsigned ? "quint16" : "qint16";
    if (base == "mediumint") return isUnsigned ? "quint32" : "qint32";
    if (base == "bigint" || base == "int8")     return isUnsigned ? "quint64" : "qint64";
    if (equalsAny({"int","integer","int4"}))    return isUnsigned ? "quint32" : "qint32";
    if (equalsAny({"serial"}))                  return "qint32";
    if (equalsAny({"bigserial"}))               return "qint64";
    if (equalsAny({"number"}) && scale == 0 && length > 0 && length <= kCfg.decimalIntegerThreshold)
        return isUnsigned ? "quint64" : "qint64";

    // REAL / DECIMAL
    if (equalsAny({"double"}) || t.contains("double precision") || equalsAny({"real"})) return "double";
    if (equalsAny({"float"})) return "float";
    if (equalsAny({"decimal","numeric","number"})) {
        if (scale == 0 && length > 0 && length <= kCfg.decimalIntegerThreshold)
            return isUnsigned ? "quint64" : "qint64";
        return "double";
    }
    if (equalsAny({"money","smallmoney"})) return "double";

    // TEXT / STRING
    if (equalsAny({"varchar","nvarchar","nchar","character varying","char","bpchar"})) return "QString";
    if (equalsAny({"text","tinytext","mediumtext","longtext","smalltext","bigtext"})) return "QString";
    if (equalsAny({"enum","set"})) return "QString";
    if (equalsAny({"json","jsonb"})) {
        return (kCfg.jsonMode == GxCodegenConfig::JsonMode::AsQString) ? "QString" : "QJsonDocument";
    }
    if (equalsAny({"uuid","uniqueidentifier","xml"})) return "QString";

    // DATE/TIME
    if (equalsAny({"year"})) return "qint32";
    if (equalsAny({"date"})) return d.contains("qodbc") ? "QDateTime" : "QDate"; // Oracle DATE -> datetime-ish
    if (base == "time")      return "QTime";
    if (equalsAny({"timestamp","datetime","timestamptz"})) return "QDateTime";

    // BINARY
    if (equalsAny({"blob","binary","varbinary","bytea","image"})) return "QByteArray";

    // NETWORK/GEOM/MISC
    if (equalsAny({"inet","cidr","macaddr","macaddr8","point","line","lseg","box","path","polygon","circle","interval"}))
        return "QString";

    // SQLite affinity fallbacks
    if (d.contains("qsqlite")) {
        if (t.contains("int")) return "qint64";
        if (t.contains("char") || t.contains("text") || t.contains("clob")) return "QString";
        if (t.contains("blob")) return "QByteArray";
        if (t.contains("real") || t.contains("floa") || t.contains("doub")) return "double";
        if (t.contains("date") && !t.contains("datetime")) return "QDate";
        if (t.contains("time") && !t.contains("datetime")) return "QTime";
        if (t.contains("datetime") || t.contains("timestamp")) return "QDateTime";
    }

    return "QString";
}

static bool writeFile(const QString& path, const QString& text, QString* err)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (err) *err = "Cannot write " + path;
        return false;
    }
    QTextStream ts(&f);
    ts.setEncoding(QStringConverter::Utf8);
    ts << text;
    if (!f.commit()) {
        if (err) *err = "Commit failed for " + path;
        return false;
    }

    return true;
}

static QString genEntityHeader(const QString& ns, const QString& entityName, const QList<ColMeta>& cols, const QString cppDriver, const QString& pkHint)
{
    bool needDT = false, needD = false, needT = false, needBA = false, needJD = false;
    QStringList fields;
    for (const auto& c : cols) {
        const QString t = mapSqlToCpp(cppDriver, c.dataType, c.length, c.scale, c.isUnsigned, /*bool*/false);
        if (t == "QDateTime") needDT = true;
        else if (t == "QDate") needD = true;
        else if (t == "QTime") needT = true;
        else if (t == "QByteArray") needBA = true;
        else if (t == "QJsonDocument") needJD = true;
        fields << (QString("    %1 %2;")).arg(t, camel(c.name));
    }

    QString out;
    out += "#pragma once\n";
    out += "// Auto-generated by GenesisX ORM codegen. Edit as needed.\n";
    out += "#include <QString>\n#include <QJsonObject>\n";
    if (needDT) out += "#include <QDateTime>\n";
    if (needD) out += "#include <QDate>\n";
    if (needT) out += "#include <QTime>\n";
    if (needBA) out += "#include <QByteArray>\n";
    if (needJD) out += "#include <QJsonDocument>\n";
    out += "\nnamespace " + ns + "{\n\n";
    out += "struct " + entityName + " {\n";
    for (const QString& f : fields) out += f + "\n";

    out += "\n  QJsonObject toJson() const {\n      QJsonObject j;\n";
    for (const auto& c : cols) {
        const QString n = c.name;
        const QString v = camel(n);
        const QString t = mapSqlToCpp(cppDriver, c.dataType, c.length, c.scale, c.isUnsigned, false);
        out += "        j[\"" + n + "\"] = ";
        if (t == "QString")         out += v;
        else if (t == "QByteArray") out += "QString::fromLatin1(" + v + ".toBase64())";
        else if (t == "QDateTime")  out += v + ".toString(Qt::ISODateWithMs)";
        else if (t == "QDate")      out += v + ".toString(Qt::ISODate)";
        else if (t == "QTime")      out += v + ".toString(Qt::ISODate)";
        else if (t == "QJsonDocument") out += v + ".toNull() ? QJsonObject{} : v.object()";
        else                        out += v;
        out += ";\n";
    }
    out += "        return j;\n     }\n";
    out += "\n    static " + entityName + " fromJson(const QJsonObject& j) {\n";
    out += "        " + entityName + " x;\n";
    for (const auto& c : cols) {
        const QString n = c.name;
        const QString v = camel(n);
        const QString t = mapSqlToCpp(cppDriver, c.dataType, c.length, c.scale, c.isUnsigned, false);
        if (t=="QString")
            out += "        x." + v + " = j.value(\""+n+"\").toString();\n";
        else if (t=="QByteArray")
            out += "        x." + v + " = QByteArray::fromBase64(j.value(\""+n+"\").toString().toLatin1());\n";
        else if (t=="QDateTime")
            out += "        x." + v + " = QDateTime::fromString(j.value(\""+n+"\").toString(), Qt::ISODateWithMs);\n";
        else if (t=="QDate")
            out += "        x." + v + " = QDate::fromString(j.value(\""+n+"\").toString(), Qt::ISODate);\n";
        else if (t=="QTime")
            out += "        x." + v + " = QTime::fromString(j.value(\""+n+"\").toString(), Qt::ISODate);\n";
        else if (t=="QJsonDocument")
            out += "        x." + v + " = QJsonDocument::fromJson(QJsonDocument(j.value(\""+n+"\").toObject()).toJson());\n";
        else if (t=="bool")
            out += "        x." + v + " = j.value(\""+n+"\").toBool(false);\n";
        else if (t=="qint8" || t=="qint16" || t=="qint32")
            out += "        x." + v + " = static_cast<" + t + ">(j.value(\""+n+"\").toInt());\n";
        else if (t=="qint64")
            out += "        x." + v + " = static_cast<qint64>(j.value(\""+n+"\").toDouble());\n";
        else if (t=="quint8" || t=="quint16" || t=="quint32")
            out += "        x." + v + " = static_cast<" + t + ">(qMax(0, j.value(\""+n+"\").toInt()));\n";
        else if (t=="quint64")
            out += "        x." + v + " = static_cast<quint64>(qMax(0, (int)j.value(\""+n+"\").toDouble()));\n";
        else if (t=="float")
            out += "        x." + v + " = static_cast<float>(j.value(\""+n+"\").toDouble());\n";
        else if (t=="double")
            out += "        x." + v + " = j.value(\""+n+"\").toDouble();\n";
        else
            out += "        /*TODO map*/ x." + v + " = " + t + "();\n";
    }
    out += "        return x;\n    }\n";

    if (!pkHint.isEmpty())
        out += "\n    // Primary key hint: " + pkHint + "\n";

    out += "};\n\n} // namespace " + ns + "\n";
    return out;
}

static QString genRepositoryFacadeHeader(const QString& ns,
                                         const QString& entityName,
                                         const QString& repoName,
                                         const QString& daName)
{
    QString out;

    out += "#pragma once\n";
    out += "#include <memory>\n";
    out += "#include <GenesisX/Orm/Repository.h>\n";
    out += "#include \"Entity/" + entityName + ".h\"\n";
    out += "#include \"DataAccess/" + daName + ".h\"\n";
    out += "#include \"Repository/api/" + repoName + "Api.h\"\n";
    out += "#include \"Repository/sql/" + repoName + "Sql.h\"\n\n";
    out += "namespace " + ns + " {\n\n";
    out += "using I" + repoName + " = gx::orm::IRepository<" + entityName + ">;\n\n";
    out += "inline std::unique_ptr<I" + repoName + "> make" + repoName + "(const " + daName + "& da) {\n";
    out += "    if (da.backend() == gx::orm::IDataAccess::Backend::Api) {\n";
    out += "        return std::unique_ptr<I" + repoName + ">(std::make_unique<" + repoName + "Api>(da).release());\n";
    out += "    }\n";
    out += "    return std::unique_ptr<I" + repoName + ">(std::make_unique<" + repoName + "Sql>(da).release());\n";
    out += "}\n\n";
    out += "} // namespace " + ns + "\n";

    return out;
}

static QString genRepositoryApiHeader(const QString& ns,
                                      const QString& entityName,
                                      const QString& repoName,
                                      const QString& daName)
{
    QString out;

    out += "#pragma once\n";
    out += "#include <GenesisX/Orm/Repository.h>\n";
    out += "#include \"Entity/" + entityName + ".h\"\n";
    out += "#include \"DataAccess/" + daName + ".h\"\n\n";
    out += "namespace " + ns + " {\n\n";
    out += "class " + repoName + "Api\n";
    out += "  : public gx::orm::ApiRepository<" + entityName + ">\n";
    // out += "    public gx::orm::IRepository<" + entityName + ">\n";
    out += "{\n";
    out += "public:\n";
    out += "    explicit " + repoName + "Api(const " + daName + "& da)\n";
    out += "        : gx::orm::ApiRepository<" + entityName + ">(da) {}\n\n";
    out += "    std::optional<" + entityName + "> find(const QVariantMap& where) override;\n";
    out += "    std::optional<" + entityName + "> findOneById(const QVariant& id) override;\n";
    out += "    QList<" + entityName + "> findAll(const QVariantMap& where, int limit=-1, int offset=0) override;\n";
    out += "    bool save(const " + entityName + "& entity) override;\n";
    out += "    bool saveAll(const QList<" + entityName + ">& entities) override;\n";
    out += "};\n\n";
    out += "} // namespace " + ns + "\n";

    return out;
}

static QString genRepositoryApiCpp(const QString& ns,
                                   const QString& entityName,
                                   const QString& repoName)
{
    QString out;

    out += "#include \"Repository/api/" + repoName + "Api.h\"\n";
    out += "#include <QJsonObject>\n";
    out += "#include <QJsonValue>\n\n";
    out += "using T = " + ns + "::" + entityName + ";\n\n";
    out += "namespace " + ns + " {\n\n";
    out += "static QJsonObject toJson(const QVariantMap& where) {\n";
    out += "    QJsonObject j; for (auto it=where.begin(); it!=where.end(); ++it)\n";
    out += "        j[it.key()] = QJsonValue::fromVariant(it.value());\n";
    out += "    return j;\n";
    out += "}\n\n";
    out += "std::optional<T> " + repoName + "Api::find(const QVariantMap& where) {\n";
    out += "    auto list = getList(toJson(where), 1, 0, nullptr);\n";
    out += "    if (list.isEmpty()) return std::nullopt;\n";
    out += "    return list.front();\n";
    out += "}\n\n";
    out += "std::optional<T> " + repoName + "Api::findOneById(const QVariant& id) {\n";
    out += "    return getOne(id.toString(), nullptr);\n";
    out += "}\n\n";
    out += "QList<T> " + repoName + "Api::findAll(const QVariantMap& where, int limit, int offset) {\n";
    out += "    return getList(toJson(where), limit, offset, nullptr);\n";
    out += "}\n\n";
    out += "bool " + repoName + "Api::save(const T& entity) {\n";
    out += "    // Default: POST to collection. Override if you prefer PUT on existing IDs.\n";
    out += "    return postOne(entity, nullptr);\n";
    out += "}\n\n";
    out += "bool " + repoName + "Api::saveAll(const QList<T>& entities) {\n";
    out += "    bool ok = true; for (const auto& e : entities) ok = postOne(e, nullptr) && ok; return ok;\n";
    out += "}\n\n";
    out += "} // namespace " + ns + "\n";

    return out;
}

static QString genRepositorySqlHeader(const QString& ns,
                                      const QString& entityName,
                                      const QString& repoName,
                                      const QString& daName,
                                      const QString& pkNameHint)
{
    QString out;

    out += "#pragma once\n";
    out += "#include <GenesisX/Orm/Repository.h>\n";
    out += "#include \"Entity/" + entityName + ".h\"\n";
    out += "#include \"DataAccess/" + daName + ".h\"\n\n";
    out += "namespace " + ns + " {\n\n";
    out += "class " + repoName + "Sql\n";
    out += "  : public gx::orm::SqlRepository<" + entityName + ">\n";
    // out += "    public gx::orm::IRepository<" + entityName + ">\n";
    out += "{\n";
    out += "public:\n";
    out += "    explicit " + repoName + "Sql(const " + daName + "& da);\n\n";
    out += "    std::optional<" + entityName + "> find(const QVariantMap& where) override;\n";
    out += "    std::optional<" + entityName + "> findOneById(const QVariant& id) override;\n";
    out += "    QList<" + entityName + "> findAll(const QVariantMap& where, int limit=-1, int offset=0) override;\n";
    out += "    bool save(const " + entityName + "& entity) override;\n";
    out += "    bool saveAll(const QList<" + entityName + ">& entities) override;\n\n";
    out += "private:\n";
    out += "    QString whereSql(const QVariantMap& where, QList<QVariant>& binds) const;\n";
    out += "    " + entityName + " mapRow(const QSqlRecord& rec) const;\n";
    if (!pkNameHint.isEmpty())
        out += "    QString primaryKey() const { return QStringLiteral(\"" + pkNameHint + "\"); }\n";
    else
        out += "    QString primaryKey() const { return QStringLiteral(\"id\"); }\n";
    out += "};\n\n";
    out += "} // namespace " + ns + "\n";
    return out;
}

// helper to emit QVariant->field conversions
static QString cppFromRecordExpr(const QString& cType, const QString& colName)
{
    const QString v = "rec.value(\"" + colName + "\")";
    if (cType == "QString")        return v + ".toString()";
    if (cType == "QByteArray")     return v + ".toByteArray()";
    if (cType == "QDateTime")      return v + ".toDateTime()";
    if (cType == "QDate")          return v + ".toDate()";
    if (cType == "QTime")          return v + ".toTime()";
    if (cType == "QJsonDocument")  return "QJsonDocument::fromJson(" + v + ".toString().toUtf8())";
    if (cType == "bool")           return v + ".toBool()";
    if (cType == "qint8" || cType == "qint16" || cType == "qint32")  return "static_cast<" + cType + ">(" + v + ".toInt())";
    if (cType == "qint64")         return v + ".toLongLong()";
    if (cType == "quint8" || cType == "quint16" || cType == "quint32") return "static_cast<" + cType + ">(" + v + ".toUInt())";
    if (cType == "quint64")        return "static_cast<quint64>(" + v + ".toULongLong())";
    if (cType == "float")          return "static_cast<float>(" + v + ".toDouble())";
    if (cType == "double")         return v + ".toDouble()";
    return cType + "()"; // fallback
}

static QString genRepositorySqlCpp(const QString& ns,
                                   const QString& entityName,
                                   const QString& repoName,
                                   const QString& daName,
                                   const QString& pkNameHint,
                                   const QList<ColMeta>& cols,
                                   const QString& cppDriver,
                                   const QString& flavorName)
{
    Q_UNUSED(daName);
    const QString pkCol = pkNameHint.isEmpty()
    ? (cols.isEmpty() ? QStringLiteral("id") : cols.first().name)
    : pkNameHint;

    // Build mapRow body
    QString mapBody;
    mapBody += "    " + ns + "::" + entityName + " x;\n";
    for (const auto& c : cols) {
        const QString t = mapSqlToCpp(cppDriver, c.dataType, c.length, c.scale, c.isUnsigned, /*isBool*/false);
        const QString field = camel(c.name);
        mapBody += "    x." + field + " = " + cppFromRecordExpr(t, c.name) + ";\n";
    }
    mapBody += "    return x;\n";

    // Build INSERT (all columns) using QStringLiteral everywhere
    QString insCols = "QStringLiteral(\"(\")";
    for (int i = 0; i < cols.size(); ++i) {
        insCols += " + q(\"" + cols[i].name + "\")";
        if (i + 1 < cols.size())
            insCols += " + QStringLiteral(\",\")";
    }
    insCols += " + QStringLiteral(\")\")";

    QString insVals = "QStringLiteral(\"(\")";
    for (int i = 0; i < cols.size(); ++i) {
        insVals += " + QStringLiteral(\"?\")";
        if (i + 1 < cols.size())
            insVals += " + QStringLiteral(\",\")";
    }
    insVals += " + QStringLiteral(\")\")";

    QString out;
    out += "#include \"Repository/sql/" + repoName + "Sql.h\"\n";
    out += "#include <QSqlQuery>\n";
    out += "#include <QSqlRecord>\n";
    out += "#include <QSqlError>\n";
    out += "#include <QVariant>\n\n";
    out += "using T = " + ns + "::" + entityName + ";\n\n";
    out += "namespace " + ns + " {\n\n";

    // ctor signature matches header (uses daName)
    out += repoName + "Sql::" + repoName + "Sql(const " + entityName + "DataAccess& da)\n";
    out += "    : gx::orm::SqlRepository<" + entityName + ">(\n";
    out += "          da,\n";
    out += "          QStringLiteral(\"" + flavorName + "\"),\n";
    out += "          da.tableName(),\n";
    out += "          [this](const QSqlRecord& r){ return mapRow(r); }\n";
    out += "      ) {}\n\n";


    out += "QString " + repoName + "Sql::whereSql(const QVariantMap& where, QList<QVariant>& binds) const {\n";
    out += "    QStringList parts;\n";
    out += "    for (auto it=where.begin(); it!=where.end(); ++it) {\n";
    out += "        parts << (q(it.key()) + QStringLiteral(\" = ?\"));\n";
    out += "        binds << it.value();\n";
    out += "    }\n";
    out += "    return parts.isEmpty()? QString() : (QStringLiteral(\" WHERE \") + parts.join(QStringLiteral(\" AND \")));\n";
    out += "}\n\n";

    // find()
    out += "std::optional<T> " + repoName + "Sql::find(const QVariantMap& where) {\n";
    out += "    QList<QVariant> binds;\n";
    out += "    const QString qtName = qt();\n";
    out += "    QString sql = QStringLiteral(\"SELECT * FROM \") + qtName + whereSql(where, binds) + ";
    if (flavorName == "MSSQL")
        out += "QStringLiteral(\" OFFSET 0 ROWS FETCH NEXT 1 ROWS ONLY\");\n";
    else
        out += "QStringLiteral(\" LIMIT 1\");\n";
    out += "    QSqlQuery q(db()); q.prepare(sql);\n";
    out += "    for (const auto& v : binds) q.addBindValue(v);\n";
    out += "    if (!gx::orm::IDataAccess::exec(q)) return std::nullopt;\n";
    out += "    if (!q.next()) return std::nullopt;\n";
    out += "    return mapRow(q.record());\n";
    out += "}\n\n";

    // findOneById()
    out += "std::optional<T> " + repoName + "Sql::findOneById(const QVariant& id) {\n";
    out += "    const QString qtName = qt();\n";
    out += "    const QString where = QStringLiteral(\" WHERE \") + q(\"" + pkCol + "\") + QStringLiteral(\"=?\");\n";
    out += "    const QString sqlBase = QStringLiteral(\"SELECT * FROM \") + qtName + where + ";
    if (flavorName == "MSSQL")
        out += "QStringLiteral(\" OFFSET 0 ROWS FETCH NEXT 1 ROWS ONLY\");\n";
    else
        out += "QStringLiteral(\" LIMIT 1\");\n";
    out += "    QSqlQuery q(db());\n";
    out += "    q.prepare(sqlBase);\n";
    out += "    q.addBindValue(id);\n";
    out += "    if (!gx::orm::IDataAccess::exec(q)) return std::nullopt;\n";
    out += "    if (!q.next()) return std::nullopt;\n";
    out += "    return mapRow(q.record());\n";
    out += "}\n\n";

    // findAll()
    out += "QList<T> " + repoName + "Sql::findAll(const QVariantMap& where, int limit, int offset) {\n";
    out += "    QList<T> outv; QList<QVariant> binds;\n";
    out += "    const QString qtName = qt();\n";
    out += "    QString sql = QStringLiteral(\"SELECT * FROM \") + qtName + whereSql(where, binds);\n";
    out += "    if (limit >= 0) {\n";
    if (flavorName == "MSSQL") {
        out += "        if (offset < 0) offset = 0;\n";
        out += "        sql += QStringLiteral(\" OFFSET \") + QString::number(offset) + QStringLiteral(\" ROWS FETCH NEXT \") + QString::number(limit) + QStringLiteral(\" ROWS ONLY\");\n";
    } else {
        out += "        sql += QStringLiteral(\" LIMIT \") + QString::number(limit);\n";
        out += "        if (offset > 0) sql += QStringLiteral(\" OFFSET \") + QString::number(offset);\n";
    }
    out += "    }\n";
    out += "    QSqlQuery q(db()); q.prepare(sql);\n";
    out += "    for (const auto& v : binds) q.addBindValue(v);\n";
    out += "    if (!gx::orm::IDataAccess::exec(q)) return outv;\n";
    out += "    while (q.next()) outv.push_back(mapRow(q.record()));\n";
    out += "    return outv;\n";
    out += "}\n\n";

    // save()
    out += "bool " + repoName + "Sql::save(const T& entity) {\n";
    out += "    const QString qtName = qt();\n";
    out += "    QString sql = QStringLiteral(\"INSERT INTO \") + qtName + " + insCols + " + QStringLiteral(\" VALUES \") + " + insVals + " + QStringLiteral(\";\");\n";
    out += "    QSqlQuery q(db()); q.prepare(sql);\n";
    for (const auto& c : cols) {
        const QString field = camel(c.name);
        out += "    q.addBindValue(QVariant::fromValue(entity." + field + "));\n";
    }
    out += "    return gx::orm::IDataAccess::exec(q);\n";
    out += "}\n\n";

    // saveAll()
    out += "bool " + repoName + "Sql::saveAll(const QList<T>& entities) {\n";
    out += "    bool ok = true; for (const auto& e : entities) ok = save(e) && ok; return ok;\n";
    out += "}\n\n";

    // mapRow()
    out += entityName + " " + repoName + "Sql::mapRow(const QSqlRecord& rec) const {\n";
    out += mapBody;
    out += "}\n\n";

    out += "} // namespace " + ns + "\n";
    return out;
}


// static QString genRepositorySqlCpp(const QString& ns,
//                                    const QString& entityName,
//                                    const QString& repoName,
//                                    const QString& pkNameHint,
//                                    const QList<ColMeta>& cols,
//                                    const QString& cppDriver,
//                                    const QString& flavorName)
// {
//     const QString pkCol = pkNameHint.isEmpty()
//         ? (cols.isEmpty() ? QStringLiteral("id") : cols.first().name)
//         : pkNameHint;

//     // Build mapRow body
//     QString mapBody;

//     mapBody += "    " + ns + "::" + entityName + " x;\n";
//     for (const auto& c : cols) {
//         const QString t = mapSqlToCpp(cppDriver, c.dataType, c.length, c.scale, c.isUnsigned, /*isBool*/false);
//         const QString field = camel(c.name);
//         mapBody += "    x." + field + " = " + cppFromRecordExpr(t, c.name) + ";\n";
//     }
//     mapBody += "    return x;\n";

//     // Build INSERT (all columns)
//     QString insCols = "\"(\"";
//     for (int i = 0; i < cols.size(); ++i) {
//         insCols += " + q(\"" + cols[i].name + "\")";
//         if (i + 1 < cols.size())
//             insCols += " + \",\"";
//     }
//     insCols += " + \")\"";

//     QString insVals = "\"(\"";
//     for (int i = 0; i < cols.size(); ++i) {
//         insVals += " + QStringLiteral(\"?\")";
//         if (i + 1 < cols.size())
//             insVals += " + \",\"";
//     }
//     insVals += " + \")\"";

//     // MS SQL uses TOP/OFFSET; we already handle LIMIT/OFFSET in queries; INSERT is standard.

//     QString out;

//     out += "#include \"Repository/sql/" + repoName + "Sql.h\"\n";
//     out += "#include <QSqlQuery>\n";
//     out += "#include <QSqlRecord>\n";
//     out += "#include <QSqlError>\n";
//     out += "#include <QVariant>\n\n";
//     out += "using T = " + ns + "::" + entityName + ";\n\n";
//     out += "namespace " + ns + " {\n\n";
//     out += repoName + "Sql::" + repoName + "Sql(const " + entityName + "DataAccess& da)\n";
//     out += "    : gx::orm::SqlRepository<" + entityName + ">(da,\n";
//     out += "        [this](const QSqlRecord& r){ return mapRow(r); }) {}\n\n";
//     out += "QString " + repoName + "Sql::whereSql(const QVariantMap& where, QList<QVariant>& binds) const {\n";
//     out += "    QStringList parts;\n";
//     out += "    for (auto it=where.begin(); it!=where.end(); ++it) {\n";
//     out += "        parts << (q(it.key()) + QStringLiteral(\" = ?\"));\n";
//     out += "        binds << it.value();\n";
//     out += "    }\n";
//     out += "    return parts.isEmpty()? QString() : (QStringLiteral(\" WHERE \") + parts.join(QStringLiteral(\" AND \")));\n";
//     out += "}\n\n";
//     out += "std::optional<T> " + repoName + "Sql::find(const QVariantMap& where) {\n";
//     out += "    QList<QVariant> binds;\n";
//     out += "    const QString qtName = dataAccess().qualifiedTable(db());\n";
//     out += "    QString sql = QStringLiteral(\"SELECT * FROM \") + qtName + whereSql(where, binds) + ";
//     if (flavorName == "MSSQL")
//         out += "QStringLiteral(\" OFFSET 0 ROWS FETCH NEXT 1 ROWS ONLY\");\n";
//     else
//         out += "QStringLiteral(\" LIMIT 1\");\n";
//     out += "    QSqlQuery q(db()); q.prepare(sql);\n";
//     out += "    for (const auto& v : binds) q.addBindValue(v);\n";
//     out += "    if (!gx::orm::IDataAccess::exec(q)) return std::nullopt;\n";
//     out += "    if (!q.next()) return std::nullopt;\n";
//     out += "    return mapRow(q.record());\n";
//     out += "}\n\n";
//     out += "std::optional<T> " + repoName + "::findOneById(const QVariant& id)\n{\n";
//     out += "    const QString qt = qt();\n";
//     out += "    const QString where = QStringLiteral(\" WHERE \") + q(\"" + pkCol + "\") + QStringLiteral(\"=?\");\n";
//     // before the line that fails
//     const QString limitOne =
//         (flavorName == "MSSQL")
//             ? "QStringLiteral(\" OFFSET 0 ROWS FETCH NEXT 1 ROWS ONLY\")"
//             : "QStringLiteral(\" LIMIT 1\")";

//     out += "    const QString sql = QStringLiteral(\"SELECT * FROM \") + qt + where + "
//            + limitOne +
//            " + QStringLiteral(\";\");\n";

//     out += "    QSqlQuery q(db());\n";
//     out += "    q.prepare(sql);\n";
//     out += "    q.addBindValue(id);\n";
//     out += "    if (!gx::orm::IDataAccess::exec(q)) return std::nullopt;\n";
//     out += "    if (!q.next()) return std::nullopt;\n";
//     out += "    return mapRow(q.record());\n";
//     out += "}\n\n";

//     out += "QList<T> " + repoName + "Sql::findAll(const QVariantMap& where, int limit, int offset) {\n";
//     out += "    QList<T> outv; QList<QVariant> binds;\n";
//     out += "    const QString qtName = dataAccess().qualifiedTable(db());\n";
//     out += "    QString sql = QStringLiteral(\"SELECT * FROM \") + qtName + whereSql(where, binds);\n";
//     out += "    if (limit >= 0) {\n";
//     if (flavorName == "MSSQL") {
//         out += "        if (offset < 0) offset = 0;\n";
//         out += "        sql += QStringLiteral(\" OFFSET \") + QString::number(offset) + QStringLiteral(\" ROWS FETCH NEXT \") + QString::number(limit) + QStringLiteral(\" ROWS ONLY\");\n";
//     } else {
//         out += "        sql += QStringLiteral(\" LIMIT \") + QString::number(limit);\n";
//         out += "        if (offset > 0) sql += QStringLiteral(\" OFFSET \") + QString::number(offset);\n";
//     }
//     out += "    }\n";
//     out += "    QSqlQuery q(db()); q.prepare(sql);\n";
//     out += "    for (const auto& v : binds) q.addBindValue(v);\n";
//     out += "    if (!gx::orm::IDataAccess::exec(q)) return outv;\n";
//     out += "    while (q.next()) outv.push_back(mapRow(q.record()));\n";
//     out += "    return outv;\n";
//     out += "}\n\n";
//     out += "bool " + repoName + "Sql::save(const T& entity) {\n";
//     out += "    const QString qtName = dataAccess().qualifiedTable(db());\n";
//     out += "    QString sql = QStringLiteral(\"INSERT INTO \") + qtName + " + insCols + " + QStringLiteral(\" VALUES \") + " + insVals + " + QStringLiteral(\";\");\n";
//     out += "    QSqlQuery q(db()); q.prepare(sql);\n";
//     for (const auto& c : cols) {
//         const QString field = camel(c.name);
//         out += "    q.addBindValue(QVariant::fromValue(entity." + field + "));\n";
//     }
//     out += "    return gx::orm::IDataAccess::exec(q);\n";
//     out += "}\n\n";
//     out += "bool " + repoName + "Sql::saveAll(const QList<T>& entities) {\n";
//     out += "    bool ok = true; for (const auto& e : entities) ok = save(e) && ok; return ok;\n";
//     out += "}\n\n";
//     out += entityName + " " + repoName + "Sql::mapRow(const QSqlRecord& rec) const {\n";
//     out += mapBody;
//     out += "}\n\n";
//     out += "} // namespace " + ns + "\n";

//     return out;
// }

static QString genDataAccessHeader(const QString& ns, const QString& daName, const QString flavorName, const QString& databaseName,
                                   const QString& schemaName, const QString& tableName, const QString baseRoute = QString{}, const QString resource = QString{})
{
    // QString connName = (flavorName == "MySql" ? "MySql" :
    //                     flavorName == "PostgreSQL" ? "PostgreSQL":
    //                     flavorName == "MSSQL" ? "MSSQL" :
    //                     flavorName == "SQLite"? "SQLite" :
    //                     flavorName == "Oracle" ? "Oracle" : "Unknown");
    QString out;

    out += "#pragma once\n";
    out += "#include <GenesisX/Orm/DataAccess.h>\n\n";
    out += "namespace " + ns + " {\n\n";
    out += "class " + daName + " : public gx::orm::IDataAccess {\n";
    out += "public:\n";
    out += "    explicit " + daName + "(Backend b = Backend::Sql)\n";
    out += "        : m_backend(b) {}\n\n";
    out += "    Backend backend()      const override { return m_backend; }\n";
    out += "    QString tableName()    const override { return \"" + tableName + "\"; }\n";
    out += "    QString connectionName() const override { return \"" + flavorName + "\"; }\n";
    out += "    QString databaseName() const override { return \"" + databaseName + "\"; }\n";
    if (!schemaName.isEmpty())
        out += "    QString schemaName()   const override { return \"" + schemaName + "\"; }\n";
    if (!baseRoute.isEmpty())
        out += "    QString baseRoute()    const override { return \"" + baseRoute + "\"; }\n";
    if (!resource.isEmpty())
        out += "    QString resourceName() const override { return \"" + resource  + "\"; }\n";
    out += "private:\n";
    out += "    Backend m_backend;\n";
    out += "};\n\n} // namespace " + ns + "\n";

    return out;
}

QString generateModel(const QSqlDatabase& db, const QString& databaseName, const QString& schemaName, const QString& tableName, const QString& outDir, const QString& ns)
{
    if (!db.isValid() || !db.isOpen())
        return "Database is not open";

    const DbFlavor flavor = detectFlavorSmart(db);
    setDbContext(db, flavor, databaseName, schemaName);

    // Cheap probe (0 rows) to validate table visibility
    QSqlQuery probe(db);
    const QString qualified = qQualified(flavor, databaseName, schemaName, tableName);
    if (!probe.exec("SELECT * FROM " + qualified + " WHERE 1=0")) {
        return "Probe failed for table " + tableName + ": " + probe.lastError().text();
    }

    // Rich metadata
    const auto cols = fetchColumns(db, flavor, databaseName, schemaName, tableName);
    if (cols.isEmpty()) return "No columns found for table " + tableName;

    const QString pk = detectPrimaryKey(db, flavor, databaseName, schemaName, tableName);

    // Class names
    const QString cls = pascal(tableName);
    const QString entityName = cls;
    const QString repoName   = cls + "Repository";
    const QString daName     = cls + "DataAccess";

    const QString flavorName = (flavor==DbFlavor::MySQL?"MySql":
                                    flavor==DbFlavor::PostgreSQL?"PostgreSQL":
                                    flavor==DbFlavor::MSSQL?"MSSQL":
                                    flavor==DbFlavor::SQLite?"SQLite":
                                    flavor==DbFlavor::Oracle?"Oracle":"Unknown");

    const QString e = genEntityHeader(ns, entityName, cols, db.driverName(), pk);
    // const QString r = genRepositoryHeader(ns, entityName, repoName, flavorName, tableName, pk);
    const QString d = genDataAccessHeader(ns, daName, flavorName, databaseName, schemaName, tableName /*, baseRoute, resourceName*/);

    const QString repoFacadeH = genRepositoryFacadeHeader(ns, entityName, repoName, daName);
    const QString repoApiH    = genRepositoryApiHeader(ns, entityName, repoName, daName);
    const QString repoApiCpp  = genRepositoryApiCpp(ns, entityName, repoName);
    const QString repoSqlH    = genRepositorySqlHeader(ns, entityName, repoName, daName, pk);
    const QString repoSqlCpp  = genRepositorySqlCpp(ns, entityName, repoName, daName, pk, cols, db.driverName(), flavorName);


    // Write files
    QString err;
    if (!writeFile(outDir + "/Entity/"     + entityName + ".h", e, &err)) return err;
    // if (!writeFile(outDir + "/Repository/" + repoName   + ".h", repoFacadeH, &err)) return err;
    if (!writeFile(outDir + "/DataAccess/"         + daName     + ".h", d, &err)) return err;

    if (!writeFile(outDir + "/Repository/"        + repoName + ".h", repoFacadeH, &err)) return err;
    if (!writeFile(outDir + "/Repository/api/"    + repoName + "Api.h", repoApiH, &err)) return err;
    if (!writeFile(outDir + "/Repository/api/"    + repoName + "Api.cpp", repoApiCpp, &err)) return err;
    if (!writeFile(outDir + "/Repository/sql/"    + repoName + "Sql.h", repoSqlH, &err)) return err;
    if (!writeFile(outDir + "/Repository/sql/"    + repoName + "Sql.cpp", repoSqlCpp, &err)) return err;

    return {};
}

QString generateModelWithConn(const QString& driverName, const QString& host, int port, const QString& user, const QString& pass, const QString& databaseName, const QString& schemaName, const QString& tableName, const QString& outDir, const QString& ns, const QString& flavorOverride)
{
    QString conn;
    const QString connName = QStringLiteral("gx_codegen_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));

    QSqlDatabase db = QSqlDatabase::addDatabase(driverName, connName);
    const QString d = driverName.toLower();

    auto getFlavorKV = [&](const QString& key) -> QString {
        const auto parts = flavorOverride.split(';', Qt::SkipEmptyParts);
        for (const QString& kv : parts) {
            const int eq = kv.indexOf('=');
            const QString k = (eq<0 ? kv : kv.left(eq)).trimmed().toLower();
            const QString v = (eq<0 ? QString() : kv.mid(eq+1)).trimmed();
            if (k == key.toLower()) return v;
        }
        return {};
    };

    if (d.contains("qodbc")) {
        const QString flv = getFlavorKV("").isEmpty() ? flavorOverride.trimmed().toLower() : flavorOverride.split(';').first().trimmed().toLower();

        const QString odbcDriverOverride = getFlavorKV("odbcdriver");

        if (host.startsWith("DSN=", Qt::CaseInsensitive)) {
            QString conn = host + ";Uid=" + user + ";Pwd=" + pass;
            if (!databaseName.isEmpty()) conn += ";Database=" + databaseName;
            db.setDatabaseName(conn);
        } else {
            const bool isMySQL = flv == "mysql" || port == 3306;
            const bool isMSSQL = flv == "mssql" || port == 1433;
            Q_UNUSED(isMSSQL)
            if (isMySQL) {
                const QString drv = !odbcDriverOverride.isEmpty()
                    ? odbcDriverOverride
                    : detectOdbcMysqlDriver();
                conn = QStringLiteral("Driver={%1};Server=%2;Port=%3;Database=%4;User=%5;Password=%6;OPTION=3;")
                    .arg(drv, host)
                    .arg(port)
                           .arg(databaseName, user, pass);
            } else {
                const QString drv = !odbcDriverOverride.isEmpty()
                    ? odbcDriverOverride
                    : QStringLiteral("ODBC Driver 18 for SQL Server");
                conn = QStringLiteral("Driver={%1};Server=tcp:%2,%3;Database=%4;Uid=%5;Pwd=%6;Encrypt=no;TrustServerCertificate=yes;")
                    .arg(drv, host)
                    .arg(port)
                    .arg(databaseName, user, pass);
            }
            db.setDatabaseName(conn);
        }
    } else if (d.contains("qsqlite")) {
        // For SQLite, 'host' is the file path
        db.setDatabaseName(host);
    } else {
        if (!host.isEmpty()) db.setHostName(host);
        if (port > 0) db.setPort(port);
        db.setUserName(user);
        db.setPassword(pass);
        if (!databaseName.isEmpty()) db.setDatabaseName(databaseName);
    }

    if (!db.open()) {
        const QString e = db.lastError().text();
        QSqlDatabase::removeDatabase(connName);
        return "Open failed here: " + e + " conn: " + conn;
    }

    QString res;
    {
        std::optional<DbFlavor> forced = parseFlavorOverride(flavorOverride);
        Q_UNUSED(forced);
        res = gx::orm::codegen::generateModel(db, databaseName, schemaName, tableName, outDir, ns);
        db.close();
    }
    QSqlDatabase::removeDatabase(connName);


    return res;
}

}
