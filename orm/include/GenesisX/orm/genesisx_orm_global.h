// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 The Kooijman Incorporate Holding B.V.

#ifndef GENESISX_ORM_GLOBAL_H
#define GENESISX_ORM_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(GENESISX_ORM_STATIC)
#  define GENESISX_ORM_EXPORT
#elif defined(GENESISX_ORM_LIBRARY)
#  define GENESISX_ORM_EXPORT Q_DECL_EXPORT
#else
#  define GENESISX_ORM_EXPORT Q_DECL_IMPORT
#endif

namespace gx { namespace orm { } }
namespace GXOrm = gx::orm;
namespace gx { namespace orm { namespace codegen { } } }
namespace GXOrmCodeGen = gx::orm::codegen;

#endif // GENESISX_ORM_GLOBAL_H
