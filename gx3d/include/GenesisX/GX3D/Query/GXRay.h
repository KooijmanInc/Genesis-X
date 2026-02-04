// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXRAY_H
#define GXRAY_H

#include <QVector3D>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

namespace gx::gx3d::query {

struct GXRay {
    QVector3D originWS;
    QVector3D dirWS;
};

}

#endif // GXRAY_H
