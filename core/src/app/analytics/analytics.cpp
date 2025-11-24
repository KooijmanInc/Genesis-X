// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "analytics.h"

/*!
    \class gx::app::analytics::Analytics
    \inmodule GenesisX
    \ingroup app-classes
    \since Qt 6.10
    \brief Minimal Analytics facade (constructor-only).

    \note Linked QML module: \c GenesisX\App\Analytics 1.0 (singleton).
    \note Enabled when the app uses qmake flag \c genesisx_app_analytics
 */
using namespace gx::app::analytics;

Analytics::Analytics(QObject* parent)
    : QObject{parent}
{}
