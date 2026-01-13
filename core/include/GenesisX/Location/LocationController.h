// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef LOCATIONCONTROLLER_H
#define LOCATIONCONTROLLER_H

#include <QObject>
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>

#include <GenesisX/genesisx_global.h>

namespace gx::app::location {

class GENESISX_CORE_EXPORT LocationController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool available READ available NOTIFY availabilityChanged)
    Q_PROPERTY(bool requesting READ requesting NOTIFY requestingChanged)

public:
    explicit LocationController(QObject* parent = nullptr);

    Q_INVOKABLE void requestOnce();
    Q_INVOKABLE void cancel();

    bool available() const;
    bool requesting() const { return m_requesting; }

signals:
    void availabilityChanged();
    void requestingChanged();

    void locationReceived(double latitude, double longitude, double accuracyMeters);
    void locationError(const QString& reason);

private:
    void handlePosition(const QGeoPositionInfo& info);
    void handleError(const QString& reason);

private:
    class QGeoPositionInfoSource* m_source = nullptr;
    bool m_requesting = false;
};

}

#endif // LOCATIONCONTROLLER_H
