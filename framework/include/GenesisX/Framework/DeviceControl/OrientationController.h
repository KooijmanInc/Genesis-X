// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef ORIENTATIONCONTROLLER_H
#define ORIENTATIONCONTROLLER_H

#include <QObject>

#include <GenesisX/Framework/genesisx_framework_global.h>

namespace gx::framework::devicecontrol {

class OrientationController : public QObject
{
    Q_OBJECT

public:
    enum Orientation {
        Portrait,
        Landscape
    };
    Q_ENUM(Orientation);

    explicit OrientationController(QObject* parent = nullptr);

    Q_INVOKABLE void toPortrait();
    Q_INVOKABLE void toLandscape();

signals:
    void orientationChangeRequested(Orientation target);
    void orientationChangeDone(Orientation target);

private slots:
    void onScreenOrientationChanged(Qt::ScreenOrientation o);

private:
    Orientation m_pending = Landscape;
    bool m_hasPending = false;
};

}

#endif // ORIENTATIONCONTROLLER_H
