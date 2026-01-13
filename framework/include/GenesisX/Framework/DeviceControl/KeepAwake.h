// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef KEEPAWAKE_H
#define KEEPAWAKE_H

#include <QObject>

#include <GenesisX/Framework/genesisx_framework_global.h>

namespace gx::framework {

class GENESISX_FRAMEWORK_EXPORT KeepAwake : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit KeepAwake(QObject* parent = nullptr);

    bool enabled() const { return m_enabled; }

public slots:
    void setEnabled(bool on);

signals:
    void enabledChanged();

private:
    bool m_enabled = false;
    void applyPlatform(bool on);
};

}

#endif // KEEPAWAKE_H
