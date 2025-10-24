// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <QNetworkReply>
#include <QByteArray>
#include <QString>

#include "genesisx_orm_global.h"

namespace gx::orm {

struct GENESISX_ORM_EXPORT HttpResponse
{
    int status = 0;
    QByteArray body;
    QString errorstring;
    QNetworkReply::NetworkError netError = QNetworkReply::NoError;
    QList<QSslError> sslErrors;
    QHash<QByteArray, QByteArray> headers;

    bool ok() const { return status >= 200 && status < 300 && netError == QNetworkReply::NoError; }
};

}

#endif // HTTPRESPONSE_H
