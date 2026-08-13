// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/Crypto/ConfigCrypto.h>

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <memory>

namespace gx::orm::crypto {

namespace {

static constexpr qsizetype KeySize = 32;   // AES-256
static constexpr qsizetype NonceSize = 12; // Recommended GCM nonce size
static constexpr qsizetype TagSize = 16;   // 128-bit authentication tag

using CipherContext = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

CipherContext createCipherContext()
{
    return CipherContext(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
}

}

QByteArray gx::orm::crypto::ConfigCrypto::generateKey()
{
    QByteArray key(KeySize, Qt::Uninitialized);

    if (RAND_bytes(
            reinterpret_cast<unsigned char*>(key.data()),
            static_cast<int>(key.size())
        ) != 1) {
        return {};
    }

    return key;
}

std::optional<EncryptionResult> ConfigCrypto::encrypt(const QByteArray &plaintext, const QByteArray &key, const QByteArray &context)
{
    if (key.size() != KeySize) {
        return std::nullopt;
    }

    EncryptionResult result;
    result.nonce.resize(NonceSize);
    result.tag.resize(TagSize);
    result.ciphertext.resize(plaintext.size());

    if (RAND_bytes(
            reinterpret_cast<unsigned char*>(result.nonce.data()),
            static_cast<int>(result.nonce.size())
        ) != 1) {
        return std::nullopt;
    }

    CipherContext cipherContext = createCipherContext();

    if (!cipherContext) {
        return std::nullopt;
    }

    if (EVP_EncryptInit_ex(
            cipherContext.get(),
            EVP_aes_256_gcm(),
            nullptr,
            nullptr,
            nullptr
        ) != 1) {
        return std::nullopt;
    }

    if (EVP_CIPHER_CTX_ctrl(
            cipherContext.get(),
            EVP_CTRL_GCM_SET_IVLEN,
            static_cast<int>(result.nonce.size()),
            nullptr
            ) != 1) {
        return std::nullopt;
    }

    if (EVP_EncryptInit_ex(
            cipherContext.get(),
            nullptr,
            nullptr,
            reinterpret_cast<const unsigned char*>(key.constData()),
            reinterpret_cast<const unsigned char*>(
                result.nonce.constData()
                )
            ) != 1) {
        return std::nullopt;
    }

    int outputLength = 0;

    if (!context.isEmpty()) {
        if (EVP_EncryptUpdate(
                cipherContext.get(),
                nullptr,
                &outputLength,
                reinterpret_cast<const unsigned char*>(
                    context.constData()
                    ),
                static_cast<int>(context.size())
                ) != 1) {
            return std::nullopt;
        }
    }

    int ciphertextLength = 0;

    if (!plaintext.isEmpty()) {
        if (EVP_EncryptUpdate(
                cipherContext.get(),
                reinterpret_cast<unsigned char*>(
                    result.ciphertext.data()
                    ),
                &outputLength,
                reinterpret_cast<const unsigned char*>(
                    plaintext.constData()
                    ),
                static_cast<int>(plaintext.size())
                ) != 1) {
            return std::nullopt;
        }

        ciphertextLength = outputLength;
    }

    if (EVP_EncryptFinal_ex(
            cipherContext.get(),
            reinterpret_cast<unsigned char*>(
                result.ciphertext.data()
                ) + ciphertextLength,
            &outputLength
            ) != 1) {
        return std::nullopt;
    }

    ciphertextLength += outputLength;
    result.ciphertext.resize(ciphertextLength);

    if (EVP_CIPHER_CTX_ctrl(
            cipherContext.get(),
            EVP_CTRL_GCM_GET_TAG,
            static_cast<int>(result.tag.size()),
            result.tag.data()
            ) != 1) {
        return std::nullopt;
    }

    return result;
}

std::optional<QByteArray> ConfigCrypto::decrypt(const EncryptionResult &encrypted, const QByteArray &key, const QByteArray &context)
{
    if (key.size() != KeySize
        || encrypted.nonce.size() != NonceSize
        || encrypted.tag.size() != TagSize) {
        return std::nullopt;
    }

    CipherContext cipherContext = createCipherContext();

    if (!cipherContext) {
        return std::nullopt;
    }

    if (EVP_DecryptInit_ex(
            cipherContext.get(),
            EVP_aes_256_gcm(),
            nullptr,
            nullptr,
            nullptr
            ) != 1) {
        return std::nullopt;
    }

    if (EVP_CIPHER_CTX_ctrl(
            cipherContext.get(),
            EVP_CTRL_GCM_SET_IVLEN,
            static_cast<int>(encrypted.nonce.size()),
            nullptr
            ) != 1) {
        return std::nullopt;
    }

    if (EVP_DecryptInit_ex(
            cipherContext.get(),
            nullptr,
            nullptr,
            reinterpret_cast<const unsigned char*>(key.constData()),
            reinterpret_cast<const unsigned char*>(
                encrypted.nonce.constData()
                )
            ) != 1) {
        return std::nullopt;
    }

    int outputLength = 0;

    if (!context.isEmpty()) {
        if (EVP_DecryptUpdate(
                cipherContext.get(),
                nullptr,
                &outputLength,
                reinterpret_cast<const unsigned char*>(
                    context.constData()
                    ),
                static_cast<int>(context.size())
                ) != 1) {
            return std::nullopt;
        }
    }

    QByteArray plaintext(
        encrypted.ciphertext.size(),
        Qt::Uninitialized
        );

    int plaintextLength = 0;

    if (!encrypted.ciphertext.isEmpty()) {
        if (EVP_DecryptUpdate(
                cipherContext.get(),
                reinterpret_cast<unsigned char*>(plaintext.data()),
                &outputLength,
                reinterpret_cast<const unsigned char*>(
                    encrypted.ciphertext.constData()
                    ),
                static_cast<int>(encrypted.ciphertext.size())
                ) != 1) {
            return std::nullopt;
        }

        plaintextLength = outputLength;
    }

    if (EVP_CIPHER_CTX_ctrl(
            cipherContext.get(),
            EVP_CTRL_GCM_SET_TAG,
            static_cast<int>(encrypted.tag.size()),
            const_cast<char*>(encrypted.tag.constData())
            ) != 1) {
        return std::nullopt;
    }

    const int status = EVP_DecryptFinal_ex(
        cipherContext.get(),
        reinterpret_cast<unsigned char*>(plaintext.data())
            + plaintextLength,
        &outputLength
        );

    // Authentication failed: wrong key/context or modified data.
    if (status != 1) {
        return std::nullopt;
    }

    plaintextLength += outputLength;
    plaintext.resize(plaintextLength);

    return plaintext;
}

}