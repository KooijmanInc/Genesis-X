// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXCASTCONTROL_H
#define GXCASTCONTROL_H

#include <QObject>

#include <GenesisX/genesisx_global.h>

namespace gx::app::cast {

class GENESISX_CORE_EXPORT GXCastControl : public QObject
{
    Q_OBJECT
public:
    explicit GXCastControl(QObject *parent = nullptr) : QObject{parent} {}

    Q_INVOKABLE void load(const QString& url, const QString& contentType, const QString& title, bool autoplay = true);
    Q_INVOKABLE void queueLoadJson(const QJsonArray& list);

signals:
    void trackFinished(const QString& url);
};

}

#endif // GXCASTCONTROL_H
