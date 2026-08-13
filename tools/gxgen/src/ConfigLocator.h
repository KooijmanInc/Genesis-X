// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef CONFIGLOCATOR_H
#define CONFIGLOCATOR_H

#include <QString>

#include "ConsoleQuestions.h"
#include "JsonFile.h"

class ConfigLocator
{
public:
    ConfigLocator();

    QString m_configurationFile = "";
    bool m_hasConfigurationFile = false;

    bool locateConfiguration(const bool useGlobal);
    bool createConfiguration();

private:
    JsonFile jsonFileHandler;
    ConsoleQuestions consoleQuestions;

    QString projectConfigPath;
    QString globalConfigPath;
};

#endif // CONFIGLOCATOR_H
