// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 The Kooijman Incorporate Holding B.V.
#ifndef GENESISX_PHYSICS_GLOBAL_H
#define GENESISX_PHYSICS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(GENESISX_PHYSICS_STATIC)
#  define GENESISX_PHYSICS_EXPORT
#elif defined(GENESISX_PHYSICS_LIBRARY)
#  define GENESISX_PHYSICS_EXPORT Q_DECL_EXPORT
#else
#  define GENESISX_PHYSICS_EXPORT Q_DECL_IMPORT
#endif

#endif // GENESISX_PHYSICS_GLOBAL_H
