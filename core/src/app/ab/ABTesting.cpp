// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "ABTesting.h"

/*!
  \class gx::app::ab::ABTesting
  \inmodule GenesisX
  \ingroup genesisx-core
  \since 6.10
  \brief Minimal A/B testing facade (constructor-only).

  This class will serve as the bridge to the app's experimentation system (e.g., a simplein-house variant allocator or a remote a/b provider).

  \note Linked QML module: \c GenesisX\App\ABTesting 1.0 (singleton).
  \note Enabled when the app uses qmake flag \c genesisx_app_ab
 */
using namespace gx::app::ab;

ABTesting::ABTesting(QObject *parent)
    : QObject{parent}
{
}
