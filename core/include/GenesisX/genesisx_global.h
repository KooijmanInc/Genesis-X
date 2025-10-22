// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 The Kooijman Incorporate Holding B.V.
#ifndef GENESISX_GLOBAL_H
#define GENESISX_GLOBAL_H

#include <QtQmlIntegration/qqmlintegration.h>
#include <QtCore/qglobal.h>

#if defined(GENESISX_CORE_STATIC)
#  define GENESISX_CORE_EXPORT
#elif defined(GENESISX_CORE_LIBRARY)
#  define GENESISX_CORE_EXPORT Q_DECL_EXPORT
#else
#  define GENESISX_CORE_EXPORT Q_DECL_IMPORT
#endif

namespace gx { }
namespace GX = gx;
namespace gx { namespace navigation { } }
namespace GXNav = gx::navigation;

#endif // GENESISX_GLOBAL_H
