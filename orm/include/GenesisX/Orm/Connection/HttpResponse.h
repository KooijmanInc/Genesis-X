// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <QByteArray>
#include <QList>
#include <QMultiHash>
#include <QNetworkReply>
#include <QString>

#include <GenesisX/Orm/Core/genesisx_orm_global.h>

namespace gx::orm {

struct GENESISX_ORM_EXPORT HttpResponse
{
    int statusCode = 0;
    QByteArray body;
    QString errorString;
    int latencyMs = -1;

    QNetworkReply::NetworkError networkError =
        QNetworkReply::NoError;

    QMultiHash<QByteArray, QByteArray> headers;

    bool ok() const
    {
        return networkError == QNetworkReply::NoError
               && statusCode >= 200
               && statusCode < 300;
    }

    QList<QByteArray> headerValues(
        const QByteArray &name
        ) const
    {
        QList<QByteArray> result;

        for (
            auto iterator = headers.constBegin();
            iterator != headers.constEnd();
            ++iterator
            ) {
            if (
                iterator.key().compare(
                    name,
                    Qt::CaseInsensitive
                    ) == 0
                ) {
                result.append(iterator.value());
            }
        }

        return result;
    }

    QByteArray header(
        const QByteArray &name
        ) const
    {
        const QList<QByteArray> values =
            headerValues(name);

        return values.isEmpty()
                   ? QByteArray{}
                   : values.first();
    }

    QString contentType() const
    {
        return QString::fromLatin1(
            header(
                QByteArrayLiteral("Content-Type")
                )
            );
    }

    bool isJson() const
    {
        const QString type =
            contentType().toLower();

        return type.startsWith(
                   QStringLiteral("application/json")
                   )
               || type.startsWith(
                   QStringLiteral("text/json")
                   )
               || body.startsWith('{')
               || body.startsWith('[');
    }
};

}

#endif // HTTPRESPONSE_H