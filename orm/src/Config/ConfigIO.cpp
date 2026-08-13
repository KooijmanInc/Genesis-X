// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/Config/ConfigIO.h>
#include <GenesisX/Orm/Config/TransportConfig.h>
#include <GenesisX/Orm/Core/AbstractConfig.h>

namespace gx::orm {

bool loadTransportConfig(const AbstractConfig &config, TransportConfig &out, const QString &language)
{
    Q_UNUSED(config)
    Q_UNUSED(out)
    Q_UNUSED(language)

    if (config.backend() == Backend::Synchronized) {
        qDebug() << "sync";
    }

    return true;
}

}