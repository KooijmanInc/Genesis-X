// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef BIOMETRICS_H
#define BIOMETRICS_H

#include <QVariant>
#include <QObject>
#include <QString>

#include <GenesisX/genesisx_global.h>

namespace gx::app::biometrics {

struct BiometricsResult {
    enum Code {
        Ok = 0,
        NotAvailable,
        UserCanceled,
        AuthFailed,
        TemporarilyUnavailable,
        KeyPermenentlyInvalidated,
        OSDenied,
        Internal
    };
    Code code = Internal;
    QString message;
};

class GENESISX_CORE_EXPORT Biometrics : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool available READ available NOTIFY availabilityChanged)
    Q_PROPERTY(bool hasLoginToken READ hasLoginToken NOTIFY loginTokenChanged)

public:
    explicit Biometrics(QObject* parent = nullptr);

    Q_INVOKABLE bool available() const;
    Q_INVOKABLE int status() const;
    Q_INVOKABLE QVariant authenticate(const QString& reason = QString());

    Q_INVOKABLE bool hasLoginToken() const;
    Q_INVOKABLE QVariant storeLoginToken(const QString& token, const QString& reason = QStringLiteral("Enable biometric login"));
    Q_INVOKABLE QVariant loadLoginToken(const QString& reason = QStringLiteral("Unlock to sign in"));
    Q_INVOKABLE bool clearLoginToken();

    bool m_tokenOpInFlight = false;

signals:
    void availabilityChanged();
    void authenticated(int code, const QString& message);

    void loginTokenChanged();
    void loginTokenReady(int code, const QString& message, const QString& token);

private:
    bool m_available = false;
    void updateAvailability();
};

}

#endif // BIOMETRICS_H
