#include "GXPlatformQml.h"
#include <GenesisX/Background/gx_background_audio.h>
#include <QtQml/qqml.h>
#include <QQmlEngine>

using namespace gx::app::background;

void GXPlatform::gx_enableBackgroundAudio() {
    gx::background::gx_enableBackgroundAudio();
}

void GXPlatform::gx_stopBackgroundServiceIfAndroid() {
    gx::background::gx_stopForegroundServiceIfAndroid();
}

void registerGenesisXBackground(QQmlEngine* engine)
{
    Q_UNUSED(engine);

    qmlRegisterSingletonType<gx::app::background::GXPlatform>("GenesisX.App.Background", 1, 0, "Background", [](QQmlEngine*, QJSEngine*) -> QObject* { return new GXPlatform(); });
}
