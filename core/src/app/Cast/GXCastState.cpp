// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "GXCastState.h"

using namespace gx::app::cast;

GXCastState *GXCastState::instance()
{
    static GXCastState s;
    return &s;
}
