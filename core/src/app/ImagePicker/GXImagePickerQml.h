// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXIMAGEPICKERQML_H
#define GXIMAGEPICKERQML_H

#include <QtQml/qqml.h>

#include <GenesisX/genesisx_global.h>

class QQmlEngine;

namespace gx::app::imagepicker {

GENESISX_CORE_EXPORT void registerGenesisXImagePicker(QQmlEngine* engine);

}

#endif // GXIMAGEPICKERQML_H
