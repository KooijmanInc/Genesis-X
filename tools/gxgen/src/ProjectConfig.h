// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef PROJECTCONFIG_H
#define PROJECTCONFIG_H

#include <QJsonObject>
#include <QString>

#include <GenesisX/Orm/Utils/Json.h>

class ProjectConfig
{
public:
    ProjectConfig(const QString& configHeaderFile);

    bool loadConfigurationPayload();
    bool createConfigurationPayload(const QString& currentContent);

    QJsonObject m_configurationPayload;

private:
    QString m_configHeaderFile;

    GXOrm::Json jsonHelper;

    QJsonObject api(bool overrides = false);
    QJsonObject sql(bool overrides = false);
};

#endif // PROJECTCONFIG_H
