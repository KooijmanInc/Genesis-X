// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Location/LocationController.h>

#include <QCoreApplication>

using namespace Qt::StringLiterals;

namespace gx::app::location {


LocationController::LocationController(QObject *parent)
    : QObject{parent}
{
    qDebug() << "Qt libraryPaths:" << QCoreApplication::libraryPaths();
    qDebug() << "Position sources available:" << QGeoPositionInfoSource::availableSources();

    m_source = QGeoPositionInfoSource::createDefaultSource(this);

    qDebug() << "Default source:" << (m_source ? m_source->sourceName() : "nullptr");
}

void LocationController::requestOnce()
{
    if (!m_source) {
        emit locationError(u"Location service unavailable"_s);
        return;
    }

    if (m_requesting) return;

    m_requesting = true;
    emit requestingChanged();

    m_source->setPreferredPositioningMethods(QGeoPositionInfoSource::AllPositioningMethods);

    connect(m_source, &QGeoPositionInfoSource::positionUpdated, this, &LocationController::handlePosition, Qt::UniqueConnection);

    connect(m_source, &QGeoPositionInfoSource::errorOccurred, this, [this](QGeoPositionInfoSource::Error e) {
        Q_UNUSED(e);
        handleError(u"Failed to obtain location"_s);
    });

    m_source->requestUpdate(5000);
}

void LocationController::cancel()
{
    if (!m_source || !m_requesting)
        return;

    m_source->stopUpdates();
    m_requesting = false;
    emit requestingChanged();
}

bool LocationController::available() const
{
    return m_source != nullptr;
}

void LocationController::handlePosition(const QGeoPositionInfo &info)
{
    if (!info.isValid()) {
        handleError(u"Invalid location data"_s);
        return;
    }

    m_source->stopUpdates();

    m_requesting = false;
    emit requestingChanged();

    const auto coord = info.coordinate();
    emit locationReceived(coord.latitude(), coord.longitude(), info.attribute(QGeoPositionInfo::HorizontalAccuracy));
}

void LocationController::handleError(const QString &reason)
{
    if (m_source) m_source->stopUpdates();

    m_requesting = false;
    emit requestingChanged();

    emit locationError(reason);
}

}
