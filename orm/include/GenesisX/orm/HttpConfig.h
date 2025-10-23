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

    bool allowInsecureDev = false;
};

}

#endif // HTTPCONFIG_H
