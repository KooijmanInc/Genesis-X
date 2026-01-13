// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 The Kooijman Incorporate Holding B.V.

#ifndef GENESISX_FRAMEWORK_GLOBAL_H
#define GENESISX_FRAMEWORK_GLOBAL_H

#include <QtCore/qglobal.h>

#if Q_QDOC
#  define GENESISX_FRAMEWORK_EXPORT
#elif defined(GENESISX_FRAMEWORK_STATIC)
#  define GENESISX_FRAMEWORK_EXPORT
#elif defined(GENESISX_FRAMEWORK_LIBRARY)
#  define GENESISX_FRAMEWORK_EXPORT Q_DECL_EXPORT
#else
#  define GENESISX_FRAMEWORK_EXPORT Q_DECL_IMPORT
#endif

/*!
    \namespace gx::framework
    \title gx::framework Namespace
    \brief Module-level APIs.
 */
namespace gx { namespace framework { } }
namespace GXFramework = gx::framework;

#endif // GENESISX_FRAMEWORK_GLOBAL_H
