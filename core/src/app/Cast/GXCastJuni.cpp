// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "GXCastState.h"

#ifdef Q_OS_ANDROID
#include <jni.h>
#endif
#include <QtCore/QString>
#include <QtCore/QMetaObject>

#ifdef Q_OS_ANDROID
static QString j2q(JNIEnv* env, jstring s) {
    if (!s) return {};
    const jchar* chars = env->GetStringChars(s, nullptr);
    jsize len = env->GetStringLength(s);
    QString out = QString::fromUtf16(reinterpret_cast<const char16_t*>(chars), len);
    env->ReleaseStringChars(s, chars);

    return out;
}

extern "C" JNIEXPORT void JNICALL
Java_com_genesisx_cast_GXCastBridge_onConnectionChanged(JNIEnv* env, jclass, jboolean connected, jstring jname)
{
    const bool isConntected = connected == JNI_TRUE;
    const QString name = j2q(env, jname);
    QMetaObject::invokeMethod(gx::app::cast::GXCastState::instance(), [=]() {
        gx::app::cast::GXCastState::instance()->setConnected(isConntected, name);
    }, Qt::QueuedConnection);
}
#endif
