// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef HTTPCONFIG_H
#define HTTPCONFIG_H

#include "genesisx_orm_global.h"
#include <QString>
#include <QHash>
#include <QUrl>

namespace gx::orm {

struct GENESISX_ORM_EXPORT HttpConfig
{
    QUrl baseUrl;
    QString appVersion;
    QString userLanguage = "en-US";
    int timeoutMs = 15000;
    int retryCount = 0;
    QHash<QByteArray, QByteArray> defaultHeaders;
    QString appKey;
    QString apiToken;
    QString bearerToken;
    bool hasAuth() const { return !appKey.isEmpty() || !apiToken.isEmpty() || !bearerToken.isEmpty(); }

    bool allowInsecureDev = false;

    bool enableDnsFallback = false;
    QString dnsIpFallback;
    int dnsRetryDelayMs = 300;
};

}

#endif // HTTPCONFIG_H
