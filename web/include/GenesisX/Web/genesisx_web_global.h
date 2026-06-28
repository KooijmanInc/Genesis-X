// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GENESISX_WEB_GLOBAL_H
#define GENESISX_WEB_GLOBAL_H

#include <QtCore/qglobal.h>

#if Q_QDOC
#  define GENESISX_WEB_EXPORT
#elif defined(GENESISX_WEB_STATIC)
#  define GENESISX_WEB_EXPORT
#elif defined(GENESISX_WEB_LIBRARY)
#  define GENESISX_WEB_EXPORT Q_DECL_EXPORT
#else
#  define GENESISX_WEB_EXPORT Q_DECL_IMPORT
#endif

/*!
    \namespace gx::web
    \title gx::web Namespace
    \brief Module-level APIs.
 */

namespace gx { namespace web { } }
namespace GXWeb = gx::web;

#endif // GENESISX_WEB_GLOBAL_H
