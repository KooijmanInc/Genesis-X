// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Framework/DeviceControl/OrientationController.h>

#include <QCoreApplication>
#include <QScreen>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif
#ifdef Q_OS_IOS
extern "C" void gx_ios_set_orientation_portrait(void);
extern "C" void gx_ios_set_orientation_landscape(void);
extern "C" void gx_ios_clear_orientation_override(void);
#endif

using namespace gx::framework::devicecontrol;

static bool isPortrait(Qt::ScreenOrientation o)
{
    return o == Qt::PortraitOrientation || o == Qt::InvertedPortraitOrientation;
}

static bool isLandscape(Qt::ScreenOrientation o)
{
    return o == Qt::LandscapeOrientation || o == Qt::InvertedLandscapeOrientation;
}

OrientationController::OrientationController(QObject *parent)
    : QObject{parent}
{
    if (QScreen *s = QGuiApplication::primaryScreen()) {
        connect(s, &QScreen::orientationChanged, this, &OrientationController::onScreenOrientationChanged);
    }
}

void OrientationController::toPortrait()
{
    m_pending = Portrait;
    m_hasPending = true;
    emit orientationChangeRequested(Portrait);

#ifdef Q_OS_ANDROID
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    if (!activity.isValid()) return;

    const int SCREEN_ORIENTATION_PORTRAIT = 1;
    activity.callMethod<void>("setRequestedOrientation", "(I)V", SCREEN_ORIENTATION_PORTRAIT);
#endif
#ifdef Q_OS_IOS
    gx_ios_set_orientation_portrait();
#endif
}

void OrientationController::toLandscape()
{
    m_pending = Landscape;
    m_hasPending = true;
    emit orientationChangeRequested(Landscape);

#ifdef Q_OS_ANDROID
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    if (!activity.isValid()) return;

    const int SCREEN_ORIENTATION_LANDSCAPE = 6;
    activity.callMethod<void>("setRequestedOrientation", "(I)V", SCREEN_ORIENTATION_LANDSCAPE);
#endif
#ifdef Q_OS_IOS
    gx_ios_set_orientation_landscape();
#endif
}

void OrientationController::onScreenOrientationChanged(Qt::ScreenOrientation o)
{
    if (!m_hasPending) return;

    const bool ok = (m_pending == Portrait && isPortrait(o)) || (m_pending == Landscape && isLandscape(o));

    if (ok) {
        m_hasPending = false;
        emit orientationChangeDone(m_pending);
    }
}
