// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/ImagePicker/GXImagePicker.h>

#include <QGuiApplication>
#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

using namespace gx::app::imagepicker;

static NativePhotoPicker* s_instance = nullptr;

NativePhotoPicker::NativePhotoPicker(QObject *parent)
    : QObject{parent}
{
    s_instance = this;
}

#ifdef Q_OS_ANDROID
static QJniObject androidContext()
{
    return QNativeInterface::QAndroidApplication::context();
}
#endif

void NativePhotoPicker::takePhoto()
{
#ifdef Q_OS_ANDROID
    auto ctx = androidContext();
    if (!ctx.isValid()) {
        emit error("Android context not available");
        return;
    }

    QJniObject::callStaticMethod<void>(
        "com/genesisx/photopicker/GXPhotoPicker",
        "openCamera",
        "(Landroid/content/Context;)V",
        ctx.object()
    );
#endif
}

void NativePhotoPicker::pickFromGallery()
{
#ifdef Q_OS_ANDROID
    auto ctx = androidContext();
    if (!ctx.isValid()) {
        emit error("Android context not available");
        return;
    }

    QJniObject::callStaticMethod<void>(
        "com/genesisx/photopicker/GXPhotoPicker",
        "openGallery",
        "(Landroid/content/Context;)V",
        ctx.object()
    );
#endif
}

#ifdef Q_OS_ANDROID
extern "C" JNIEXPORT void JNICALL
Java_com_genesisx_photopicker_GXPhotoPickerActivity_onImageReady(JNIEnv*, jclass, jstring path)
{
    if (!s_instance) return;

    QJniObject jPath(path);

    const QString filePath = jPath.toString();

    emit s_instance->imageReady(QUrl::fromLocalFile(filePath));
}

extern "C" JNIEXPORT void JNICALL
Java_com_genesisx_photopicker_GXPhotoPickerActivity_onError(JNIEnv*, jclass, jstring message)
{
    if (!s_instance) return;

    QJniObject jPath(message);

    const QString errorMessage = jPath.toString();

    emit s_instance->error(errorMessage);
}
#endif
