// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/Config/ConfigIO.h>
#include <GenesisX/Orm/Config/TransportConfig.h>
#include <GenesisX/Orm/Core/AbstractConfig.h>

#include <GenesisX/Orm/Crypto/ConfigCrypto.h>

#include <QJsonObject>

namespace gx::orm {

inline QByteArray configKey(const AbstractConfig& config)
{
    return config.configurationKey();
}

bool loadTransportConfig(const AbstractConfig &config, TransportConfig &out, const QString& env, const QString &language)
{
    Q_UNUSED(out)
    Q_UNUSED(language)
    Json jsonHelper;

    const QByteArray context = config.configurationContext();

    const QByteArray key = configKey(config);

    auto apiSql = jsonHelper.getJsonObjectFromByteArray(config.configurationPayload());

    if (config.backend() == Backend::Synchronized) {
        qDebug() << "use sync";
    } else if (config.backend() == Backend::Http) {
        if (apiSql.contains("api")) {
            const QJsonValue apiValue = apiSql.value(QStringLiteral("api"));
            if (!apiValue.isString()) {
                qCritical()
                << "[GX ORM] API configuration is missing";

                return false;
            }
            const QByteArray encryptedApiPayload = QByteArray::fromBase64(apiValue.toString().toLatin1());
            if (encryptedApiPayload.isEmpty()) {
                qCritical()
                << "[GX ORM] Unable to decode API configuration";

                return false;
            }

            const auto encryptedApi =
                crypto::ConfigCrypto::deserialize(
                    encryptedApiPayload
                    );

            if (!encryptedApi.has_value()) {
                qCritical()
                << "[GX ORM] Invalid encrypted API payload";

                return false;
            }

            const auto decryptedApi = crypto::ConfigCrypto::decrypt(*encryptedApi, key, context);

            if (!decryptedApi.has_value()) {
                qCritical()
                << "[GX ORM] Unable to decrypt API configuration";

                return false;
            }

            const QByteArray api =
                *decryptedApi;
            qDebug() << api;
        }
    }

    return true;
}

}