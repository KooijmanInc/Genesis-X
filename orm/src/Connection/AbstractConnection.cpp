// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Orm/Connection/AbstractConnection.h>

using namespace gx::orm;


AbstractConnection::AbstractConnection(QObject *parent)
    : QObject{parent}
{}
