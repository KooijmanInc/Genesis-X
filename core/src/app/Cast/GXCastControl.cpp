// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "GXCastControl.h"

#include <QCoreApplication>
#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif
#include <QPointer>
#include <QJsonArray>

using namespace gx::app::cast;

#ifdef Q_OS_ANDROID
static inline QJniObject activityCtx() {
    return QNativeInterface::QAndroidApplication::context();
}
#endif

void GXCastControl::load(const QString &url, const QString &contentType, const QString &title, bool autoplay)
{
    Q_UNUSED(url);
    Q_UNUSED(contentType);
    Q_UNUSED(title);
    Q_UNUSED(autoplay);
#ifdef Q_OS_ANDROID
    QJniObject ctx = activityCtx();
    if (!ctx.isValid()) return;
    QJniObject jUrl = QJniObject::fromString(url);
    QJniObject jCt = QJniObject::fromString(contentType);
    QJniObject jTitle = QJniObject::fromString(title);

    QJniObject::callStaticMethod<void>(
        "com/genesisx/cast/GXCastManager",
        "loadMedia",
        "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Z)V",
        ctx.object<jobject>(), jUrl.object<jstring>(), jCt.object<jstring>(), jTitle.object<jstring>(),(jboolean)autoplay
    );
#endif
}

void GXCastControl::queueLoadJson(const QJsonArray &list)
{
    Q_UNUSED(list);
#ifdef Q_OS_ANDROID
    QJniObject ctx = activityCtx();
    if (!ctx.isValid()) return;
    const QString json = QString::fromUtf8(QJsonDocument(list).toJson(QJsonDocument::Compact));
    QJniObject jPlaylist = QJniObject::fromString(json);

    QJniObject::callStaticMethod<void>(
        "com/genesisx/cast/GXCastManager",
        "loadMediaList",
        "(Landroid/content/Context;Ljava/lang/String;)V",
        ctx.object<jobject>(), jPlaylist.object<jstring>()
    );
#endif
}



#ifdef Q_OS_ANDROID
GENESISX_CORE_EXPORT extern GXCastControl* gx_cast_control_singleton();

extern "C" JNIEXPORT void JNICALL
Java_com_genesisx_cast_GXCastBridge_onTrackFinished(JNIEnv* env, jclass, jstring jcontentId)
{
    // convert jstring -> QString
    const jchar* chars = env->GetStringChars(jcontentId, nullptr);
    jsize len = env->GetStringLength(jcontentId);
    QString cid = QString::fromUtf16(reinterpret_cast<const char16_t*>(chars), len);
    env->ReleaseStringChars(jcontentId, chars);

    QPointer<gx::app::cast::GXCastControl> ctrl = gx_cast_control_singleton();

    QMetaObject::invokeMethod(qApp, [ctrl, cid] {
        if (!ctrl) return;
        emit ctrl->trackFinished(cid);
    });

    // QMetaObject::invokeMethod(gx_cast_control_singleton(), [cid]() {
    //     emit gx_cast_control_singleton()->trackFinished(cid);
    // }, Qt::QueuedConnection);

    // emit trackFinished(cid);

    // QMetaObject::invokeMethod(GXCastControl::instance(), [cid](){
    //     // emit a Qt signal your QML can catch
    //     emit GXCastPlayback::instance()->trackFinished(cid);
    // }, Qt::QueuedConnection);
}
#endif
