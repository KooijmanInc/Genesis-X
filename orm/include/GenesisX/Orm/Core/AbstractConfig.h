// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef ABSTRACTCONFIG_H
#define ABSTRACTCONFIG_H

#include <GenesisX/Orm/Core/genesisx_orm_global.h>
#include <GenesisX/Orm/Config/TransportConfig.h>

namespace gx::orm {

class GENESISX_ORM_EXPORT AbstractConfig
{
public:
    AbstractConfig() = default;
    virtual ~AbstractConfig() = default;

    [[nodiscard]] virtual GXOrm::Backend backend() const = 0;
};

}

#endif // ABSTRACTCONFIG_H
