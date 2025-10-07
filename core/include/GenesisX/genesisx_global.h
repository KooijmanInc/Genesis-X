// Copyright (C) 2025 The Kooijman Incorporate Holding B.V.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#ifndef GENESISX_GLOBAL_H
#define GENESISX_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(GENESISX_STATIC)
#  define GENESISX_EXPORT
#elif defined(GENESISX_LIBRARY)
#  define GENESISX_EXPORT Q_DECL_EXPORT
#else
#  define GENESISX_EXPORT Q_DECL_IMPORT
#endif

#endif // GENESISX_GLOBAL_H
