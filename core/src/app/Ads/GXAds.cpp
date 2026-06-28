// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Ads/GXAds.h>

/*!
    \class gx::app::ads::GXAds
    \inmodule io.genesisx.app
    \ingroup app-classes
    \title Ads
    \since Qt 6.10
    \brief Main class for displaying ads in app.
 */

/*!
    \qmlmodule GenesisX.Ads
    \inmodule io.genesisx.app
    \title Genesis-X Ads (QML)
    \since Qt 6.10
    \nativetype gx::app::ads::GXAds
    \brief QML APIs for displaying ads.

    Import this module to use the \l GXAds type:

    \code
    import GenesisX.Ads 1.0
    \endcode
 */

/*!
    \qmltype Ads
    \inqmlmodule io.genesisx.app
    \since Qt 6.10
    \nativetype gx::app::ads::GXAds
    \brief Handles displaying ads with unified API.

    \section2 Example
    \qml
    import GenesisX.Ads 1.0

    Ads {

    }
    \endqml
 */

/*!
    \qmlsignal NotificationHandler::initializedChanged()

    Emitted whenever the \l initialized property changes.
*/

using namespace gx::app::ads;

GXAds::GXAds(QObject *parent)
    : QObject{parent}
{

}

void GXAds::initialize(const QVariantMap& options)
{
    Q_UNUSED(options);
    if (m_initialized) return;

    m_initialized = true;
    emit initializedChanged();
}

bool GXAds::initialized() const
{
    return m_initialized;
}
