// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "BiometricsQml.h"

#include <QtQml/qqml.h>
#include <QQmlEngine>

#include "include/GenesisX/Biometrics/Biometrics.h"
#include "BiometricsQml.h"

using namespace gx::app::biometrics;

void registerGenesisXBiometrics(QQmlEngine* engine)
{
    Q_UNUSED(engine);

    qmlRegisterType<gx::app::biometrics::Biometrics>("GenesisX.App.Biometrics", 1, 0, "Biometrics");
}
