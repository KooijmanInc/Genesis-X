// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "GXCastState.h"
#include "GXCastControl.h"
#include "GXCastLifecycle.h"
#include <GenesisX/Cast/Cast.h>

#include <QQmlEngine>

using namespace gx::app::cast;

namespace {
    static Cast s_castBridge;
    static GXCastLifecycle s_lifecycle;
    static GXCastControl s_control;
}

GXCastControl* gx_cast_control_singleton() { return &s_control; }

void registerGenesisXCast(QQmlEngine *engine)
{
    Q_UNUSED(engine);

    qmlRegisterSingletonInstance<gx::app::cast::Cast>("GenesisX.Cast", 1, 0, "Cast", &s_castBridge);

    qmlRegisterSingletonInstance<gx::app::cast::GXCastState>("GenesisX.Cast", 1, 0, "CastState", gx::app::cast::GXCastState::instance());

    qmlRegisterSingletonInstance<gx::app::cast::GXCastLifecycle>("GenesisX.Cast", 1, 0, "GXCastLifecycle", &s_lifecycle);

    qmlRegisterSingletonInstance<gx::app::cast::GXCastControl>("GenesisX.Cast", 1, 0, "GXCastControl", &s_control);
}











