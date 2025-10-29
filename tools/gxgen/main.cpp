// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

#include "GenesisX/Orm/Codegen.h"

// ---- simple config helpers -------------------------------------------------
static QJsonObject loadConfig(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return {};
    const auto doc = QJsonDocument::fromJson(f.readAll());
    return doc.isObject() ? doc.object() : QJsonObject{};
}

static void saveConfig(const QString& path, const QJsonObject& j) {
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(j).toJson(QJsonDocument::Indented));
        f.close();
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser p;
    p.setApplicationDescription("GenesisX ORM model generator");
    p.addHelpOption();

    QCommandLineOption dbOpt    ({"d","database"}, "Database name", "name");
    QCommandLineOption tblOpt   ({"t","table"},    "Table name", "name");
    QCommandLineOption outOpt   ({"o","out"},      "Output dir (e.g. ../app)", "dir", ".");
    QCommandLineOption drvOpt   ({"r","driver"},   "Qt SQL driver (QMYSQL/QSQLITE/QODBC/...)", "driver", "QMYSQL");
    QCommandLineOption hostOpt  ({"H","host"},     "Host (or sqlite file for QSQLITE, or DSN=Name for ODBC)", "host", "localhost");
    QCommandLineOption portOpt  ({"P","port"},     "Port", "port", "3306");
    QCommandLineOption userOpt  ({"u","user"},     "User", "user", "root");
    QCommandLineOption passOpt  ({"p","pass"},     "Password", "pass", "");
    QCommandLineOption nsOpt    ({"n","ns"},       "C++ namespace for generated code", "ns", "App");
    QCommandLineOption schemaOpt({"s","schema"},   "Schema name (PG/MSSQL/Oracle)", "schema", "");
    QCommandLineOption flavorOpt({"F","flavor"},   "Backend flavor hint for ODBC (mysql|mssql|postgresql|sqlite|oracle)", "flavor", "");
    QCommandLineOption odbcDrvOpt({"O","odbc-driver"}, "ODBC driver display name (e.g. \"MySQL ODBC 9.5 Unicode Driver\")", "driver", "");

    p.addOption(dbOpt); p.addOption(tblOpt); p.addOption(outOpt);
    p.addOption(drvOpt); p.addOption(hostOpt); p.addOption(portOpt);
    p.addOption(userOpt); p.addOption(passOpt); p.addOption(nsOpt);
    p.addOption(schemaOpt); p.addOption(flavorOpt); p.addOption(odbcDrvOpt);

    p.process(app);

    // Early required check
    if (!p.isSet(tblOpt) || !p.isSet(dbOpt)) {
        p.showHelp(1);
    }

    // Load config.json (working dir)
    QJsonObject cfg = loadConfig("config.json");

    // Helper: CLI overrides config; fallback to default
    auto val = [&](const QCommandLineOption& opt, const char* key, const QString& def){
        if (p.isSet(opt)) return p.value(opt);
        if (cfg.contains(key)) return cfg.value(key).toString();
        return def;
    };

    const QString driver = val(drvOpt,   "driver",   "QMYSQL");
    const QString host   = val(hostOpt,  "host",     "localhost");
    const QString port   = val(portOpt,  "port",     "3306");
    const QString user   = val(userOpt,  "user",     "root");
    const QString pass   = val(passOpt,  "pass",     "");
    const QString ns     = val(nsOpt,    "ns",       "App");
    const QString schema = val(schemaOpt,"schema",   "");
    QString flavor       = val(flavorOpt,"flavor",   "");   // mutable, we may append odbcDriver
    const QString outDir = val(outOpt,   "out",      ".");

    const QString dbName = p.value(dbOpt);  // required, per check above
    const QString table  = p.value(tblOpt); // required

    // If user provided an explicit ODBC driver, append it to flavor as key=value
    if (p.isSet(odbcDrvOpt)) {
        if (!flavor.isEmpty()) flavor += ';';
        flavor += "odbcDriver=" + p.value(odbcDrvOpt);
    }

    // Run generator
    QString err = GXOrmCodeGen::generateModelWithConn(
        driver, host, port.toInt(), user, pass,
        dbName, schema, table, outDir, ns, flavor);

    // Save config (on success) for next runs (don’t store db/table)
    if (err.isEmpty()) {
        cfg["driver"] = driver;
        cfg["host"]   = host;
        cfg["port"]   = port;
        cfg["user"]   = user;
        cfg["pass"]   = pass;      // consider omitting if you don’t want to store creds
        cfg["ns"]     = ns;
        cfg["schema"] = schema;
        cfg["flavor"] = flavor;
        cfg["out"]    = outDir;
        saveConfig("config.json", cfg);
    }

    QTextStream out(stdout);
    if (!err.isEmpty()) {
        out << "ERROR: " << err << "\n";
        return 2;
    }

    out << "Generated entity/repository/dataaccess for " << table
        << " into " << outDir << "\n";
    return 0;
}


// #include <QCoreApplication>
// #include <QCommandLineParser>
// #include <QTextStream>
// #include <QJsonDocument>
// #include <QJsonObject>
// #include <QFile>

// #include "GenesisX/Orm/Codegen.h"

// static QJsonObject loadConfig(const QString& path) {
//     QFile f(path);
//     if (!f.open(QIODevice::ReadOnly)) return {};
//     const auto doc = QJsonDocument::fromJson(f.readAll());
//     return doc.isObject() ? doc.object() : QJsonObject{};
// }

// static void saveConfig(const QString& path, const QJsonObject& j) {
//     QFile f(path);
//     if (f.open(QIODevice::WriteOnly)) {
//         f.write(QJsonDocument(j).toJson(QJsonDocument::Indented));
//         f.close();
//     }
// }

// int main(int argc, char** argv)
// {
//     QCoreApplication app (argc, argv);
//     QCommandLineParser p;
//     p.setApplicationDescription("GenesisX ORM model generator");
//     p.addHelpOption();

//     QCommandLineOption dbOpt({"d","database"}, "Database name", "name");
//     QCommandLineOption tblOpt({"t","table"}, "Table name", "name");
//     QCommandLineOption outOpt({"o","out"}, "Output dir (e.g. ../app)", "dir", ".");
//     QCommandLineOption drvOpt({"r","driver"}, "Qt SQL driver (QMYSQL/QSQLITE/QODBC/...)", "driver", "QMYSQL");
//     QCommandLineOption hostOpt({"H","host"}, "Host (or sqlite file for QSQLITE, or DSN=Name for ODBC)", "host", "localhost");
//     QCommandLineOption portOpt({"P","port"}, "Port", "port", "3306");
//     QCommandLineOption userOpt({"u","user"}, "User", "user", "root");
//     QCommandLineOption passOpt({"p","pass"}, "Password", "pass", "");
//     QCommandLineOption nsOpt({"n","ns"}, "C++ namespace for generated code", "ns", "App");
//     QCommandLineOption schemaOpt({"s","schema"}, "Schema name (PG/MSSQL/Oracle)", "schema", "");
//     QCommandLineOption flavorOpt({"F","flavor"}, "Backend flavor hint when using ODBC (mysql|mssql|postgresql|sqlite|oracle)", "flavor", "");
//     QCommandLineOption odbcDrvOpt({"O","odbc-driver"}, "ODBC driver display name (e.g. \"MySQL ODBC 8.0 Unicode Driver\")", "driver", "");


//     p.addOption(dbOpt); p.addOption(tblOpt); p.addOption(outOpt);
//     p.addOption(drvOpt); p.addOption(hostOpt); p.addOption(portOpt);
//     p.addOption(userOpt); p.addOption(passOpt); p.addOption(nsOpt);
//     p.addOption(schemaOpt); p.addOption(flavorOpt); p.addOption(odbcDrvOpt);

//     p.process(app);

//     // Load config.json from working dir (or you can use outDir later)
//     QJsonObject cfg = loadConfig("config.json");

//     // Helper to pull a value: CLI overrides config; fall back to default
//     auto val = [&](const QCommandLineOption& opt, const char* key, const QString& def){
//         if (p.isSet(opt)) return p.value(opt);
//         if (cfg.contains(key)) return cfg.value(key).toString();
//         return def;
//     };

//     const QString driver = val(drvOpt,   "driver",   "QMYSQL");
//     const QString host   = val(hostOpt,  "host",     "localhost");
//     const QString port   = val(portOpt,  "port",     "3306");
//     const QString user   = val(userOpt,  "user",     "root");
//     const QString pass   = val(passOpt,  "pass",     "");
//     const QString ns     = val(nsOpt,    "ns",       "App");
//     const QString schema = val(schemaOpt,"schema",   "");
//     const QString flavor = val(flavorOpt,"flavor",   "");
//     const QString outDir = val(outOpt,   "out",      ".");
//     const QString dbName = p.value(dbOpt);     // required
//     const QString table  = p.value(tblOpt);    // required

//     QString err = gx::orm::codegen::generateModelWithConn(
//         driver, host, port.toInt(), user, pass,
//         dbName, schema, table, outDir, ns, flavor);

//     if (err.isEmpty()) {
//         cfg["driver"] = driver;
//         cfg["host"]   = host;
//         cfg["port"]   = port;
//         cfg["user"]   = user;
//         cfg["pass"]   = pass;     // optional: omit if you don’t want to store creds
//         cfg["ns"]     = ns;
//         cfg["schema"] = schema;
//         cfg["flavor"] = flavor;
//         cfg["out"]    = outDir;
//         saveConfig("config.json", cfg);
//     }

//     if (!p.isSet(tblOpt) || !p.isSet(dbOpt)) {
//         p.showHelp(1);
//     }
//     // flavor = p.value(flavorOpt);

//     // if (p.isSet(odbcDrvOpt)) {
//     //     if (!flavor.isEmpty()) flavor += ";";
//     //     flavor += "odbcDriver=" + p.value(odbcDrvOpt);
//     // }

//     // QString err = GXOrmCodeGen::generateModelWithConn(p.value(drvOpt), p.value(hostOpt), p.value(portOpt).toInt(), p.value(userOpt), p.value(passOpt), p.value(dbOpt), p.value(schemaOpt), p.value(tblOpt), p.value(outOpt), p.value(nsOpt), flavor);

//     QTextStream out(stdout);
//     if (!err.isEmpty()) { out << "ERROR: " << err << "\n"; return 2; }
//     out << "Generated entity/repository/dataaccess for " << p.value(tblOpt) << " into " << p.value(outOpt) << "\n";

//     return 0;
// }
