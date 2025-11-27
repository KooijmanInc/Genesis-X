// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef CONNECTIONCHECK_H
#define CONNECTIONCHECK_H

#include "genesisx_orm_global.h"
#include <QString>

namespace gx::orm {

struct GENESISX_ORM_EXPORT GxConnectionReport
{
    QString backend;
    bool ok = false;
    int latencyMs = -1;
    QString message;
};

class GENESISX_ORM_EXPORT IGxConnectionChecker
{
public:
    virtual ~IGxConnectionChecker() = default;
    virtual GxConnectionReport check() = 0;
};

class ConnectionController;

class GENESISX_ORM_EXPORT HttpConnectionChecker : public IGxConnectionChecker
{
public:
    explicit HttpConnectionChecker(ConnectionController* c);
    ~HttpConnectionChecker();
    GxConnectionReport check() override;

private:
    ConnectionController* m_conn = nullptr;
};

}

#endif // CONNECTIONCHECK_H
