// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GX3DQML_H
#define GX3DQML_H

#include "genesisx_gx3d_global.h"

class QQmlEngine;

namespace gx::gx3d {

GENESISX_GX3D_EXPORT void registerEnabledQmlModules(QQmlEngine* engine);

}

#endif // GX3DQML_H
