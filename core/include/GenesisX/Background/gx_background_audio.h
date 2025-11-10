// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GX_BACKGROUND_AUDIO_H
#define GX_BACKGROUND_AUDIO_H

#include <QtGlobal>

#include <GenesisX/genesisx_global.h>

namespace gx::background {

GENESISX_CORE_EXPORT void gx_enableBackgroundAudio();

GENESISX_CORE_EXPORT void gx_stopForegroundServiceIfAndroid();

}

#endif // GX_BACKGROUND_AUDIO_H
