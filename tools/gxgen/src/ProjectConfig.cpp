// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "ProjectConfig.h"

#include <GenesisX/Orm/Crypto/ConfigCrypto.h>

#include <QFile>
#include <QJsonParseError>

ProjectConfig::ProjectConfig(const QString& configHeaderFile)
{
    m_configHeaderFile = configHeaderFile;
}

bool ProjectConfig::loadConfigurationPayload()
{
    QFile file(m_configHeaderFile);

    QString content;

    if (!file.exists()) {
        content = QString(
            "#ifndef CONFIG_H\n"
            "#define CONFIG_H\n\n"
            "#include <GenesisX/Orm/Core/AbstractConfig.h>\n\n"
            "class Config final : public GXOrm::AbstractConfig\n"
            "{\n"
            "public:\n"
            "    GXOrm::Backend backend() const override;\n\n"
            "private:\n"
            "};\n\n"
            "#endif // CONFIGOLD_H"
        );
    } else {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCritical() << "Unable to open Config.h:" << m_configHeaderFile;
            return false;
        }

        content = QString::fromUtf8(file.readAll());
    }

    static const QRegularExpression payloadExpression(
        R"(QByteArray\s+m_configurationPayload\s*=\s*R"GX\(([\s\S]*?)\)GX"\s*;)"
        );

    const QRegularExpressionMatch match =
        payloadExpression.match(content);

    if (!match.hasMatch()) {
        return createConfigurationPayload(content);
    }

    QJsonParseError parseError;

    const QJsonDocument document = QJsonDocument::fromJson(
        match.captured(1).trimmed().toUtf8(),
        &parseError
        );

    if (parseError.error != QJsonParseError::NoError
        || !document.isObject()) {
        qCritical() << "Invalid configuration payload in Config.h:"
                    << parseError.errorString();
        return false;
    }

    m_configurationPayload = document.object();

    return true;
}

bool ProjectConfig::createConfigurationPayload(const QString& currentContent)
{
    Q_UNUSED(currentContent);
    QByteArray payload;
    QJsonObject base;
    QJsonObject overrides;
    QJsonObject dev;
    QJsonObject staging;
    QJsonObject production;
    QTextStream input(stdin);
    QTextStream output(stdout);
    QByteArray byte;
    const QByteArray context = "Genesis-X|AgeGap|schema=1";

    const QByteArray key = gx::orm::crypto::ConfigCrypto::generateKey();

    output << "--- Setup configuration payload ---\n";
    output << "-- Set http settings\n" << Qt::flush;

    byte = jsonHelper.getByteArrayFromJsonObject(api());
    const auto apiBytes = gx::orm::crypto::ConfigCrypto::encrypt(byte, key, context);
    base.insert("api", QString::fromLatin1(apiBytes->ciphertext.toBase64()));

    output << "-- Set sql settings\n" << Qt::flush;
    byte.clear();
    byte = jsonHelper.getByteArrayFromJsonObject(sql());
    const auto sqlBytes = gx::orm::crypto::ConfigCrypto::encrypt(byte, key, context);
    base.insert("sql", QString::fromLatin1(sqlBytes->ciphertext.toBase64()));

    output << Qt::flush << "-- Set http settings for staging\n" << Qt::flush;
    byte.clear();
    byte = jsonHelper.getByteArrayFromJsonObject(api(true));
    const auto apiStagingBytes = gx::orm::crypto::ConfigCrypto::encrypt(byte, key, context);
    staging.insert("api", QString::fromLatin1(apiStagingBytes->ciphertext.toBase64()));
    output << "-- Set sql settings for staging\n" << Qt::flush;
    byte.clear();
    byte = jsonHelper.getByteArrayFromJsonObject(sql(true));
    const auto sqlStagingBytes = gx::orm::crypto::ConfigCrypto::encrypt(byte, key, context);
    staging.insert("sql", QString::fromLatin1(sqlStagingBytes->ciphertext.toBase64()));

    overrides.insert("staging", staging);

    output << Qt::flush << "-- Set  http settings for development\n" << Qt::flush;
    byte.clear();
    byte = jsonHelper.getByteArrayFromJsonObject(api(true));
    const auto apiDevBytes = gx::orm::crypto::ConfigCrypto::encrypt(byte, key, context);
    dev.insert("api", QString::fromLatin1(apiDevBytes->ciphertext.toBase64()));
    output << "-- Set sql settings for development\n" << Qt::flush;
    byte.clear();
    byte = jsonHelper.getByteArrayFromJsonObject(sql(true));
    const auto sqlDevBytes = gx::orm::crypto::ConfigCrypto::encrypt(byte, key, context);
    dev.insert("sql", QString::fromLatin1(sqlDevBytes->ciphertext.toBase64()));

    overrides.insert("dev", dev);

    base.insert("overrides", overrides);

    const QString property = QString(
        "\n"
        "    QByteArray m_configurationPayload = R\"GX(%1)GX\";\n"
    ).arg(jsonHelper.getByteArrayFromJsonObject(base));

    QString updatedContent = currentContent;

    const qsizetype privatePosition =
        updatedContent.lastIndexOf("private:");

    if (privatePosition >= 0) {
        const qsizetype insertionPosition =
            privatePosition + QStringLiteral("private:").size();

        updatedContent.insert(insertionPosition, property);
    } else {
        const qsizetype classEnd = updatedContent.lastIndexOf("};");

        if (classEnd < 0) {
            qCritical() << "Cannot find Config class closing brace in:"
                        << m_configHeaderFile;
            return false;
        }

        updatedContent.insert(
            classEnd,
            QString("\nprivate:%1").arg(property)
            );
    }

    QFile file(m_configHeaderFile);

    if (!file.open(
            QIODevice::WriteOnly
            | QIODevice::Truncate
            | QIODevice::Text
            )) {
        qCritical() << "Unable to update Config.h:"
                    << m_configHeaderFile;
        return false;
    }

    file.write(updatedContent.toUtf8());

    // m_configurationPayload = payload;


    // QByteArray devApiByte = jsonHelper.getByteArrayFromJsonObject(dev);
    // const QByteArray context =
    //     "Genesis-X|AgeGap|development|schema=1";
    // const auto devBytes = gx::orm::crypto::ConfigCrypto::encrypt(devApiByte, key, context);
    // // qDebug() << devBytes->ciphertext;
    // payload.insert("development", QString::fromLatin1(devBytes->ciphertext.toBase64()));
    // qDebug() << payload;

    // const QByteArray encryptedPayload =
    //     QByteArray::fromBase64(
    //         configurationPayload
    //             .value("development")
    //             .toString()
    //             .toLatin1()
    //         );

    return false;
}

QJsonObject ProjectConfig::api(bool overrides) {
    QTextStream input(stdin);
    QTextStream output(stdout);
    QJsonObject api;
    output << "Base url: " << Qt::flush;
    const QString baseUrl = input.readLine().trimmed();
    api.insert("baseUrl", baseUrl);

    if (overrides == false) {
        output << "App version: " << Qt::flush;
        const QString appVersion = input.readLine().trimmed();
        api.insert("appVersion", appVersion);

        output << "Timeout ms: " << Qt::flush;
        const QString timeoutMs = input.readLine().trimmed();
        api.insert("timeoutMs", timeoutMs);

        output << "Retry count: " << Qt::flush;
        const QString retryCount = input.readLine().trimmed();
        api.insert("retryCount", retryCount);
    }
    output << "Allow insecure development certificate: " << Qt::flush;
    const QString allowInsecureDev = input.readLine().trimmed();
    api.insert("allowInsecureDev", allowInsecureDev);

    output << "App key: " << Qt::flush;
    const QString appKey = input.readLine().trimmed();
    api.insert("appKey", appKey);

    output << "Api token: " << Qt::flush;
    const QString apiToken = input.readLine().trimmed();
    api.insert("apiToken", apiToken);

    output << "Bearer token: " << Qt::flush;
    const QString bearerToken = input.readLine().trimmed();
    api.insert("bearerToken", bearerToken);

    if (overrides == false) {
        QJsonObject headers;
        output << "Headers Accept: " << Qt::flush;
        const QString accept = input.readLine().trimmed();
        headers.insert("Accept", accept);

        output << "Headers User Agent Android: " << Qt::flush;
        const QString userAgentAndroid = input.readLine().trimmed();
        headers.insert("User-Agent-android", userAgentAndroid);

        api.insert("headers", headers);

        output << "Enable dns fallback: " << Qt::flush;
        const QString enableDnsFallback = input.readLine().trimmed();
        api.insert("enableDnsFallback", enableDnsFallback);

        output << "Dns ip fallback: " << Qt::flush;
        const QString dnsIpFallback = input.readLine().trimmed();
        api.insert("dnsIpFallback", dnsIpFallback);

        output << "Dns retry delay ms: " << Qt::flush;
        const QString dnsRetryDelayMs = input.readLine().trimmed();
        api.insert("dnsRetryDelayMs", dnsRetryDelayMs);
    }

    return api;
}

QJsonObject ProjectConfig::sql(bool overrides)
{
    QTextStream input(stdin);
    QTextStream output(stdout);
    QJsonObject sql;

    if (overrides == false) {
        output << "Driver: " << Qt::flush;
        const QString driver = input.readLine().trimmed();
        sql.insert("driver", driver);
    }

    output << "Host: " << Qt::flush;
    const QString host = input.readLine().trimmed();
    sql.insert("host", host);

    output << "Port: " << Qt::flush;
    const QString port = input.readLine().trimmed();
    sql.insert("port", port);

    if (overrides == false) {
        output << "Database: " << Qt::flush;
        const QString database = input.readLine().trimmed();
        sql.insert("database", database);
    }

    output << "User: " << Qt::flush;
    const QString user = input.readLine().trimmed();
    sql.insert("user", user);

    output << "Password: " << Qt::flush;
    const QString password = input.readLine().trimmed();
    sql.insert("password", password);

    return sql;
}
