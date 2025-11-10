// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "GXPlatformQml.h"
#include <GenesisX/Background/gx_background_audio.h>
#include "GXBackgroundRouter.h"
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

    qmlRegisterSingletonInstance<gx::app::background::BackgroundMediaRouter>("GenesisX.App.BgRouter", 1, 0, "BgRouter", BackgroundMediaRouter::instance());
}
