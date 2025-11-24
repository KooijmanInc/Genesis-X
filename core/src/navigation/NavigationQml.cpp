// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "NavigationQml.h"
#include "GxRouter.h"

#include <QtQml/qqml.h>
#include <QQmlEngine>

using namespace gx::navigation;

namespace {
// constexpr auto kUri = "GenesisX.Core.Navigation";
static gx::navigation::GxRouter s_router;
}



void registerGenesisXNavigation(QQmlEngine* engine)
{
    Q_UNUSED(engine);

    qmlRegisterSingletonInstance("GenesisX.Navigation", 1, 0, "Router", &s_router);
}
