// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef AUTH_H
#define AUTH_H

#include <QObject>

#include <GenesisX/genesisx_global.h>

namespace gx::app::auth {

class GENESISX_CORE_EXPORT Auth : public QObject
{
    Q_OBJECT
public:
    explicit Auth(QObject *parent = nullptr);

signals:
};

}

#endif // AUTH_H
