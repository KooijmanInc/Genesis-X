// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef ABTESTING_H
#define ABTESTING_H

#include <QObject>

#include <GenesisX/genesisx_global.h>

namespace gx::app::ab {

class GENESISX_CORE_EXPORT ABTesting : public QObject
{
    Q_OBJECT
public:
    explicit ABTesting(QObject* parent = nullptr);
};

}

#endif // ABTESTING_H
