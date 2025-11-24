// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXCASTSTATE_H
#define GXCASTSTATE_H

#include <QObject>
#include <QString>

#include <GenesisX/genesisx_global.h>

namespace gx::app::cast {

class GENESISX_CORE_EXPORT GXCastState : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceNameChanged)

public:
    static GXCastState* instance();

    bool connected() const { return m_connected; }
    QString deviceName() const { return m_deviceName; }

public slots:
    void setConnected(bool c, const QString& name) {
        if (m_connected != c) { m_connected = c; emit connectedChanged(); }
        if (m_deviceName != name) { m_deviceName = name; emit deviceNameChanged(); }
    }

signals:
    void connectedChanged();
    void deviceNameChanged();
    void trackFinished(const QString& contentId);

private:
    explicit GXCastState(QObject *parent = nullptr) : QObject{parent} {}
    bool m_connected = false;
    QString m_deviceName;

signals:
};

}

#endif // GXCASTSTATE_H
