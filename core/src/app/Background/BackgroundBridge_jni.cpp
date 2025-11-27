// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifdef Q_OS_ANDROID
#include <jni.h>
#include <QString>
#include <QMetaObject>
#include "GXBackgroundRouter.h"

using namespace Qt::StringLiterals;

using namespace gx::app::background;

extern "C" JNIEXPORT void JNICALL
Java_com_genesisx_background_BackgroundBridge_dispatchCommand(JNIEnv* env, jclass /*clazz*/, jstring jcmd)
{
    const char* utf = env->GetStringUTFChars(jcmd, nullptr);
    QString cmd = QString::fromUtf8(utf ? utf : "");
    env->ReleaseStringUTFChars(jcmd, utf);

    auto router = BackgroundMediaRouter::instance();
    QMetaObject::invokeMethod(router, [router, cmd]() {
        emit router->rawCommand(cmd);

        if (cmd == u"play"_s) emit router->playRequested();
        else if (cmd == u"pause"_s) emit router->pauseRequested();
        else if (cmd == u"next"_s) emit router->nextRequested();
        else if (cmd == u"previous"_s) emit router->previousRequested();
    }, Qt::QueuedConnection);
}
#endif
