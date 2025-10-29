// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 The Kooijman Incorporate Holding B.V.

#ifndef GENESISX_GLOBAL_H
#define GENESISX_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(GENESISX_CORE_LIBRARY) && defined(GENESISX_CORE_STATIC)
#  error "GENESISX_CORE_LIBRARY and GENESISX_CORE_STATIC cannot both be defined."
#endif

#if Q_QDOC
#  define GENESISX_CORE_EXPORT
#elif defined(GENESISX_CORE_STATIC)
#  define GENESISX_CORE_EXPORT
#elif defined(GENESISX_CORE_LIBRARY)
#  define GENESISX_CORE_EXPORT Q_DECL_EXPORT
#else
#  define GENESISX_CORE_EXPORT Q_DECL_IMPORT
#endif

namespace gx { }
namespace GX = gx;
namespace gx { namespace core { } }
namespace GXCore = gx::core;
namespace gx { namespace navigation { } }
namespace GXNav = gx::navigation;
namespace gx { namespace utils { } }
namespace GXUtl = gx::utils;

#endif // GENESISX_GLOBAL_H
