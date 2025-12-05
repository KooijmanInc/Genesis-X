// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Background/GXBackgroundRouter.h>
#include <QPointer>

/*!
    \class gx::app::background::BackgroundMediaRouter
    \inmodule io.genesisx.app
    \ingroup app-classes
    \title Background media router
    \since Qt 6.10
    \brief Feature for play media in background of the device.
 */

using namespace gx::app::background;

static QPointer<BackgroundMediaRouter> s_inst;

BackgroundMediaRouter *BackgroundMediaRouter::instance()
{
    if (!s_inst) s_inst = new BackgroundMediaRouter();

    return s_inst;
}
