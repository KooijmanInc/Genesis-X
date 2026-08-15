// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef HTTPREQUESTSTATE_H
#define HTTPREQUESTSTATE_H

#include <QTimer>
#include <QPromise>
#include <QElapsedTimer>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <GenesisX/Orm/Core/genesisx_orm_global.h>
#include <GenesisX/Orm/Connection/HttpResponse.h>

#include <memory>

namespace gx::orm {

struct GENESISX_ORM_EXPORT HttpRequestState
{
    QPromise<HttpResponse> promise;
    QElapsedTimer elapsedTimer;
    bool timedOut = false;
};

}

#endif // HTTPREQUESTSTATE_H
