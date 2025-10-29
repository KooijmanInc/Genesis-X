// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef ANALYTICS_H
#define ANALYTICS_H

#include <QObject>

#include <GenesisX/genesisx_global.h>

namespace gx::app::analytics {

class GENESISX_CORE_EXPORT Analytics : public QObject
{
    Q_OBJECT
public:
    explicit Analytics(QObject* parent = nullptr);
};

}

#endif // ANALYTICS_H
