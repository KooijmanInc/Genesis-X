// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "GXBackgroundRouter.h"
#include <QPointer>

using namespace gx::app::background;

static QPointer<BackgroundMediaRouter> s_inst;

BackgroundMediaRouter *BackgroundMediaRouter::instance()
{
    if (!s_inst) s_inst = new BackgroundMediaRouter();

    return s_inst;
}
