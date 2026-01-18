// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 The Kooijman Incorporate Holding B.V.

#ifndef GENESISX_GX3D_GLOBAL_H
#define GENESISX_GX3D_GLOBAL_H

#include <QtCore/qglobal.h>

#if Q_QDOC
#  define GENESISX_GX3D_EXPORT
#elif defined(GENESISX_GX3D_STATIC)
#  define GENESISX_GX3D_EXPORT
#elif defined(GENESISX_GX3D_LIBRARY)
#  define GENESISX_GX3D_EXPORT Q_DECL_EXPORT
#else
#  define GENESISX_GX3D_EXPORT Q_DECL_IMPORT
#endif

namespace gx { namespace gx3d { } }
namespace GX3d = gx::gx3d;

#endif // GENESISX_GX3D_GLOBAL_H
