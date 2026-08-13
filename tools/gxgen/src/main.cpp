// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
// #include <QFileInfo>
// #include <QFile>
// #include <QDir>

// #include <GenesisX/Orm/Core/AbstractConfig.h>

#include "ProjectConfig.h"
#include "ConfigLocator.h"
#include "JsonFile.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser p;
    bool useGlobal = false;

    p.setApplicationDescription("GenesisX ORM model generator");
    p.addHelpOption();

    QCommandLineOption glbCfgOpt ({"g","global-gonfig"}, "Use global config file true/false", "bool");
    QCommandLineOption dbOpt     ({"d","database"}, "Database name", "name");
    QCommandLineOption tblOpt    ({"t","table"},    "Table name", "name");

    p.addOption(glbCfgOpt);
    p.addOption(dbOpt); p.addOption(tblOpt);

    p.process(app);

    if (!p.isSet(tblOpt) || !p.isSet(dbOpt)) {
        p.showHelp(1);
    }

    if (p.isSet(glbCfgOpt)) {
        const QString useG = p.value(glbCfgOpt);
        if (useG == "true") {
            useGlobal = true;
        }
    }

    ConfigLocator configLocator;

    if (!configLocator.locateConfiguration(useGlobal)) {
        QTextStream output(stdout);

        output << "No configuration selected, aborting...";

        return EXIT_FAILURE;
    }

    if (!configLocator.m_hasConfigurationFile) {
        if (!configLocator.createConfiguration()) {
            QTextStream output(stdout);

            output << "Creating configuration file aborted.";

            return EXIT_FAILURE;
        }
    }

    QJsonObject cfg;
    JsonFile jsonFileHandler;
    cfg = jsonFileHandler.loadJsonFile(configLocator.m_configurationFile);

    if (!cfg.contains("configHeaderFile")) {
        QTextStream output(stdout);

        output << "We could not find the correct file layout, aborting...";
    }

    ProjectConfig* projectConfig = new ProjectConfig(cfg.value("configHeaderFile").toString());
    if (!projectConfig->loadConfigurationPayload()) {
        return EXIT_FAILURE;
    }

    // qDebug() << ;
    // QJsonObject configH;



    // if (createConfigFile) {
    //     QJsonObject tmpCfg;
    //     QString createConfigPath;

    //     if (askYesNo(
    //             "Create Genesis-X app configuration in:\n"
    //             + projectConfigPath + "?"
    //             )) {
    //         createConfigPath = projectConfigPath;
    //     } else if (askYesNo(
    //                    "Create global Genesis-X configuration in:\n"
    //                    + globalConfigPath + "?"
    //                    )) {
    //         createConfigPath = globalConfigPath;
    //     } else {
    //         return EXIT_FAILURE;
    //     }

    //     QTextStream input(stdin);
    //     QTextStream output(stdout);

    //     output << "Path to Genesis-X config.h: " << Qt::flush;

    //     const QString configHeaderPath = input.readLine().trimmed();

    //     if (configHeaderPath.isEmpty()) {
    //         output << "No config.h path provided.\n";
    //         return EXIT_FAILURE;
    //     }

    //     QDir dir;
    //     QDir configPath(configHeaderPath);
    //     if (!configPath.exists()) {
    //         if (askYesNo(
    //                 "Create directory:\n"
    //                 + configHeaderPath + "?"
    //                 )) {
    //             dir.mkdir(configHeaderPath);
    //         }
    //     }

    //     const QFileInfo confFileInfo(createConfigPath);
    //     const QString confDir = confFileInfo.absolutePath();
    //     dir.mkdir(confDir);

    //     // tmpCfg = loadConfig(createConfigPath);
    //     // tmpCfg["configHeaderFile"] = configHeaderPath + "/Config.h";
    //     // saveConfig(createConfigPath, tmpCfg);
    //     // useConfigFile = createConfigPath;
    // }

    // QJsonObject cfg;
    // cfg = loadConfig(useConfigFile);
    // qDebug() << cfg;



    return EXIT_SUCCESS;
}