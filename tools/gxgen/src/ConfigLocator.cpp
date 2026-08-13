// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "ConfigLocator.h"

#include <QFileInfo>
#include <QFile>
#include <QDir>

ConfigLocator::ConfigLocator()
{
    const QString workingDirectory = QDir::currentPath();
    projectConfigPath = workingDirectory + "/.gx/config.json";
    globalConfigPath = QDir::homePath() + "/.gx/config.json";
}

bool ConfigLocator::locateConfiguration(const bool useGlobal)
{
    const bool projectConfigExists = QFileInfo::exists(projectConfigPath);
    const bool globalConfigExists = QFileInfo::exists(globalConfigPath);

    if (useGlobal) {
        if (!globalConfigExists) {
            if (!consoleQuestions.askYesNo("No global Genesis-X configuration found. Create one?")) {
                m_configurationFile = "";
                m_hasConfigurationFile = false;

                return false;
            }
            m_configurationFile = globalConfigPath;
        } else {
            m_configurationFile = globalConfigPath;
            m_hasConfigurationFile = true;

            return true;
        }
    } else if (projectConfigExists) {
        m_configurationFile = projectConfigPath;
        m_hasConfigurationFile = true;

        return true;
    } else if (globalConfigExists) {
        m_configurationFile = globalConfigPath;
        m_hasConfigurationFile = true;

        return true;
    } else {
        if (consoleQuestions.askYesNo("No Genesis-X configuration found. Create one in project directory?")) {
            m_configurationFile = projectConfigPath;
            m_hasConfigurationFile = false;

            return true;
        }
        if (consoleQuestions.askYesNo("No Genesis-X configuration found. Create one in global directory?")) {
            m_configurationFile = globalConfigPath;
            m_hasConfigurationFile = false;

            return true;
        }

        m_configurationFile = "";
        m_hasConfigurationFile = false;

        return false;
    }

    return false;
}

bool ConfigLocator::createConfiguration()
{
    QTextStream input(stdin);
    QTextStream output(stdout);

    output << "Path to Genesis-X config.h: " << Qt::flush;
    const QString configHeaderPath = input.readLine().trimmed();

    if (configHeaderPath.isEmpty()) {
        output << "No config.h path provided.\n";
        return false;
    }

    QDir dir;
    QDir configPath(configHeaderPath);
    if (!configPath.exists()) {
        if (consoleQuestions.askYesNo("Create directory:\n" + configHeaderPath + "?")) {
            dir.mkdir(configHeaderPath);
        } else {
            return false;
        }
    }

    const QFileInfo confFileInfo(m_configurationFile);
    const QString confDir = confFileInfo.absolutePath();
    dir.mkdir(confDir);

    QJsonObject cfg;

    cfg = jsonFileHandler.loadJsonFile(m_configurationFile);
    // qDebug() << m_configurationFile;
    cfg["configHeaderFile"] = configHeaderPath + "/Config.h";
    jsonFileHandler.saveJsonFile(m_configurationFile, cfg);

    return true;
}
