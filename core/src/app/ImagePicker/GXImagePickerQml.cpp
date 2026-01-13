// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "GXImagePickerQml.h"

#include <GenesisX/ImagePicker/GXImagePicker.h>
#include <GenesisX/ImagePicker/GXImageCropService.h>

#include <QtQml/qqml.h>
#include <QQmlEngine>

using namespace gx::app::imagepicker;

static NativePhotoPicker* s_instance = nullptr;

void registerGenesisXImagePicker(QQmlEngine *engine)
{
    Q_UNUSED(engine);

    if (!s_instance) {
        s_instance = new NativePhotoPicker();
    }

    qmlRegisterSingletonInstance<gx::app::imagepicker::NativePhotoPicker>("GenesisX.Images", 1, 0, "PhotoPicker", s_instance);
    qmlRegisterSingletonType<GX::app::imagepicker::GXImageCropService>("GenesisX.Images", 1, 0, "ImageCrop", [](QQmlEngine*, QJSEngine*) -> QObject* { return new GXImageCropService; });
}
