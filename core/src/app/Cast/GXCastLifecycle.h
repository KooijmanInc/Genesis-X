// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXCASTLIFECYCLE_H
#define GXCASTLIFECYCLE_H

#include <QCoreApplication>
#include <QObject>

#include <GenesisX/genesisx_global.h>

namespace gx::app::cast {

class GENESISX_CORE_EXPORT GXCastLifecycle : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit GXCastLifecycle(QObject *parent = nullptr);

    bool enabled() const { return m_enabled; }
    Q_INVOKABLE void disconnect();

private slots:
    void setEnabled(bool on);

signals:
    void enabledChanged();

private:
    void startJavaListener();
    void stopJavaListener();
    void onAppStateChanged(Qt::ApplicationState s);

    bool m_enabled = true;
    bool m_started = false;
};

}

#endif // GXCASTLIFECYCLE_H
