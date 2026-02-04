// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXAABB_H
#define GXAABB_H

#include <QVector3D>

#include <GenesisX/GX3D/genesisx_gx3d_global.h>

namespace gx::gx3d::query {

struct GXAabb {
    QVector3D minWS;
    QVector3D maxWS;
};

}

#endif // GXAABB_H
