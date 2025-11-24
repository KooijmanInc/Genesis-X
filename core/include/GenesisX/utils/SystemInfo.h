// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include <QString>

#include <GenesisX/genesisx_global.h>

namespace gx::utils {

struct GENESISX_CORE_EXPORT SystemInfo {
    static QString ensureAppUuid();
    static QString operatingSystem();
    static QString platform();
    static QString systemLanguage();
};

// class GENESISX_CORE_EXPORT SystemInfoQml
// {
// public:
//     SystemInfoQml();
// };

}

#endif // SYSTEMINFO_H
