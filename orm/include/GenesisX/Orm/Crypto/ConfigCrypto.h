// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef CONFIGCRYPTO_H
#define CONFIGCRYPTO_H

#include <GenesisX/Orm/Core/genesisx_orm_global.h>

#include <QByteArray>

#include <optional>

namespace gx::orm::crypto {

struct EncryptionResult
{
    QByteArray nonce;
    QByteArray ciphertext;
    QByteArray tag;
};

class GENESISX_ORM_EXPORT ConfigCrypto
{
public:
    static QByteArray generateKey();

    static std::optional<EncryptionResult> encrypt(const QByteArray& plaintext, const QByteArray& key, const QByteArray& context);

    static std::optional<QByteArray> decrypt(const EncryptionResult& encrypted, const QByteArray& key, const QByteArray& context);

    static QByteArray serialize(const EncryptionResult &encrypted);

    static std::optional<EncryptionResult> deserialize(const QByteArray &payload);

};

}

#endif // CONFIGCRYPTO_H
