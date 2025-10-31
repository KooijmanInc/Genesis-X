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

public:
    explicit Biometrics(QObject* parent = nullptr);

    Q_INVOKABLE bool available() const;
    Q_INVOKABLE int status() const;
    Q_INVOKABLE QVariant authenticate(const QString& reason = QString());

signals:
    void availabilityChanged();
    void authenticated(int code, const QString& message);

private:
    bool m_available = false;
    void updateAvailability();
};

}

#endif // BIOMETRICS_H
