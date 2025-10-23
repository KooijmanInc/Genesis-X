// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef VEHICLE4W_H
#define VEHICLE4W_H

#include <QObject>

#include <GenesisX/genesisx_physics_global.h>

namespace gx::physics {

class GENESISX_PHYSICS_EXPORT Vehicle4W : public QObject
{
    Q_OBJECT
public:
    explicit Vehicle4W(QObject* parent = nullptr);
};

}

#endif // VEHICLE4W_H
