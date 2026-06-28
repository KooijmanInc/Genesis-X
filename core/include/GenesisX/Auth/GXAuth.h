// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXAUTH_H
#define GXAUTH_H

#include <QObject>

#include <GenesisX/genesisx_global.h>

namespace gx::app::auth {

class GENESISX_CORE_EXPORT GXAuth : public QObject
{
    Q_OBJECT
public:
    explicit GXAuth(QObject *parent = nullptr);

signals:
};

}

#endif // GXAUTH_H
