// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef ABSTRACTCONFIG_H
#define ABSTRACTCONFIG_H

#include <GenesisX/Orm/Core/genesisx_orm_global.h>
#include <GenesisX/Orm/Config/TransportConfig.h>
// #include <GenesisX/Orm/Config/ConfigIO.h>

namespace gx::orm {

class ConfigIO;

class GENESISX_ORM_EXPORT AbstractConfig
{
public:
    AbstractConfig() = default;
    virtual ~AbstractConfig() = default;

    [[nodiscard]] virtual GXOrm::Backend backend() const = 0;
    [[nodiscard]] virtual QByteArray configurationPayload() const = 0;
    [[nodiscard]] virtual QByteArray configurationContext() const = 0;

private:
    [[nodiscard]] virtual QByteArray configurationKey() const = 0;

    friend QByteArray configKey(const AbstractConfig *config);
    // friend bool loadTransportConfig(
    //     const AbstractConfig &config,
    //     TransportConfig &out,
    //     const QString &language
    //     );
};

}

#endif // ABSTRACTCONFIG_H
