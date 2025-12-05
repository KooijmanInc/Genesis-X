// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Background/gx_background_audio.h>

/*!
    \qmlmodule GenesisX.Background
    \inqmlmodule io.genesisx.app
    \nativetype gx::app::background::BackgroundMediaRouter
    \title Genesis-X Background (QML)
    \since Qt 6.10
    \brief QML APIs for background.
 */

/*!
    \qmltype Background
    \inqmlmodule io.genesisx.app
    \nativetype gx::app::background::BackgroundMediaRouter
    \since Qt 6.10
    \brief QML APIs for background.
 */

namespace gx::background {

#ifdef Q_OS_IOS
extern "C" void gx_enableBackgroundAudio_ios();
#endif
#ifdef Q_OS_ANDROID
extern "C" void gx_startForegroundAudioService_android();
extern "C" void gx_stopForegroundAudioService_android();
#endif

/*!
    \qmlmethod Background::gx_enableBackgroundAudio()
 */
void gx_enableBackgroundAudio()
{
#ifdef Q_OS_IOS
    gx_enableBackgroundAudio_ios();
#endif
#ifdef Q_OS_ANDROID
    gx_startForegroundAudioService_android();
#endif
}

/*!
    \qmlmethod Background::gx_stopForegroundServiceIfAndroid()
 */
void gx_stopForegroundServiceIfAndroid()
{
#ifdef Q_OS_ANDROID
    gx_stopForegroundAudioService_android();
#endif
}

}
