// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "WebsiteQml.h"
#include <QQmlEngine>

#include <GenesisX/Web/GXWebsite.h>

namespace gx::web {

void registerGenesisXWebsite(QQmlEngine *engine)
{
    Q_UNUSED(engine);

    qmlRegisterType<gx::web::GXWebsite>("GenesisX.Web", 1, 0, "Website");
}

}
