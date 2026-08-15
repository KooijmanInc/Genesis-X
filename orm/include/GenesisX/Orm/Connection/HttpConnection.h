// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <QNetworkAccessManager>
#include <QUrl>

#include <GenesisX/Orm/Connection/AbstractConnection.h>
#include <GenesisX/Orm/Config/HttpConfig.h>
#include <GenesisX/Orm/Connection/HttpResponse.h>

namespace gx::orm {

class GENESISX_ORM_EXPORT HttpConnection final : public AbstractConnection
{
    Q_OBJECT

public:
    explicit HttpConnection(const HttpConfig& config, QObject* parent = nullptr);

    QFuture<ConnectionResult> ping() override;

private:
    QUrl resolveUrl(const QString &path) const;
    QFuture<HttpResponse> get(const QString& path);

    HttpConfig m_config;
    QNetworkAccessManager m_networkAccessManager;
};

}

#endif // HTTPCONNECTION_H
