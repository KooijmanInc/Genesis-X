// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXBACKGROUNDROUTER_H
#define GXBACKGROUNDROUTER_H

#include <QObject>

#include <GenesisX/genesisx_global.h>

namespace gx::app::background {

class GENESISX_CORE_EXPORT BackgroundMediaRouter : public QObject
{
    Q_OBJECT

public:
    static BackgroundMediaRouter* instance();

signals:
    void playRequested();
    void pauseRequested();
    void nextRequested();
    void previousRequested();
    void rawCommand(QString cmd);

private:
    explicit BackgroundMediaRouter(QObject* parent = nullptr) : QObject{parent} {}
};

}

#endif // GXBACKGROUNDROUTER_H
