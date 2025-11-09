#include <GenesisX/Background/gx_background_audio.h>

namespace gx::background {

#ifdef Q_OS_IOS
extern "C" void gx_enableBackgroundAudio_ios();
#endif
#ifdef Q_OS_ANDROID
extern "C" void gx_startForegroundAudioService_android();
extern "C" void gx_stopForegroundAudioService_android();
#endif

void gx_enableBackgroundAudio()
{
#ifdef Q_OS_IOS
    gx_enableBackgroundAudio_ios();
#endif
#ifdef Q_OS_ANDROID
    gx_startForegroundAudioService_android();
#endif
}

void gx_stopForegroundServiceIfAndroid()
{
#ifdef Q_OS_ANDROID
    gx_stopForegroundAudioService_android();
#endif
}

}
