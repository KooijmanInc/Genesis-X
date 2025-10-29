// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <QJsonDocument>
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

    QString contentType() const {
        auto it = headers.constFind("Content-Type");
        return (it == headers.cend()) ? QString() : QString::fromLatin1(it.value());
    }
    bool isJson() const {
        const auto ct = contentType().toLower();
        return ct.startsWith("application/json") || ct.startsWith("text/json") || body.startsWith('{') || body.startsWith('[');
    }

    // JSON accessors (lazy parsed). If not valid JSON, returns empty doc/obj/array and sets *err if provided.
    QJsonDocument jsonDoc(QString* err = nullptr) const;
    QJsonObject   jsonObject(QString* err = nullptr) const;
    QJsonArray    jsonArray(QString* err = nullptr) const;
    QVariant      jsonVariant(QString* err = nullptr) const; // object->map, array->list

private:
    // cache parsed result
    mutable bool          _jsonParsed = false;
    mutable QJsonDocument _jsonDoc;
    mutable QString       _jsonErr;
};

}

#endif // HTTPRESPONSE_H
