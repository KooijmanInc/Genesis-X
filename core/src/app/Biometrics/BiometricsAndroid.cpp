// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "include/GenesisX/Biometrics/Biometrics.h"

#include <QCoreApplication>
#ifdef Q_OS_ANDROID
#include <QJniEnvironment>
#include <QJniObject>
#include <QtCore/QJniObject>
#endif

using namespace gx::app::biometrics;

static inline jlong ptr(QObject* o) {
    return reinterpret_cast<jlong>(o);
}

static BiometricsResult::Code mapStatusToCode(int status) {
    switch (status) {
    case 0: return BiometricsResult::Ok;
    case 1: return BiometricsResult::NotAvailable;
    case 2: return BiometricsResult::TemporarilyUnavailable;
    default: return BiometricsResult::Internal;
    }
}

bool gx_app_biometrics_available_android() {
#ifdef Q_OS_ANDROID
    QJniObject res = QJniObject::callStaticObjectMethod(
        "biometrics/GxBiometrics",
        "getStatus",
        "()Ljava/lang/Integer;"
        );

    return res.isValid() && res.callMethod<jint>("intValue","()I") == 0;
#else
    return false;
#endif
}

bool gx_app_biometrics_status_android() {
#ifdef Q_OS_ANDROID
    QJniObject res = QJniObject::callStaticObjectMethod(
        "biometrics/GxBiometrics",
        "getStatus",
        "()Ljava/lang/Integer;"
        );

    if (!res.isValid()) return BiometricsResult::Internal;
    return mapStatusToCode(res.callMethod<jint>("intValue", "()I"));
#else
    return false;
#endif
}

static void emitResult(QObject* ctx, BiometricsResult::Code code, const QString& msg)
{
    auto* obj = qobject_cast<Biometrics*>(ctx);
    if (!obj) return;
    BiometricsResult r{code, msg};
    emit obj->authenticated(code, msg);
}

#ifdef Q_OS_ANDROID
extern "C" JNIEXPORT void JNICALL
Java_com_genesisx_app_biometrics_GxBiometrics_nativeOnAuthResult(JNIEnv* env, jclass, jlong ptr, jint code, jstring jmsg) {
    QString msg;
    if (jmsg) {
        const char* chars = env->GetStringUTFChars(jmsg, nullptr);
        msg = QString::fromUtf8(chars);
        env->ReleaseStringUTFChars(jmsg, chars);
    }
    QMetaObject::invokeMethod(reinterpret_cast<QObject*>(ptr), [ptr, code, msg]() {
        emitResult(reinterpret_cast<QObject*>(ptr), static_cast<BiometricsResult::Code>(code), msg);
    }, Qt::QueuedConnection);
}
#endif

QVariant gx_app_biometrics_authenticate_android(const QString& reason, QObject* ctx)
{
    Q_UNUSED(reason);
    Q_UNUSED(ctx);
#ifdef Q_OS_ANDROID
    // QJniObject activity = QNativeInterface::QAndroidApplication::
    jlong ptr = reinterpret_cast<jlong>(ctx);
    QJniObject jReason = QJniObject::fromString(reason);
    QJniObject::callStaticMethod<void>(
        "biometrics/GxBiometrics",
        "authenticate",
        "(JLjava/lang/String;)V",
        ptr, jReason.object<jstring>()
    );
#endif
    return {};
}

#ifdef Q_OS_ANDROID
extern "C" JNIEXPORT void JNICALL
Java_biometrics_GxBiometrics_notifyQt(JNIEnv* env,
                                      jclass /*clazz*/,
                                      jlong objPtr,
                                      jint code,
                                      jstring jmsg)
{
    Q_UNUSED(env);
    auto *obj = reinterpret_cast<Biometrics*>(objPtr);
    if (!obj)
        return;

    QString msg = QJniObject(jmsg).toString();

    // Ensure we emit on Qt's thread
    QMetaObject::invokeMethod(
        obj,
        [obj, code, msg]() {
            emit obj->authenticated(static_cast<int>(code), msg);
            if (code == 0 && msg == QStringLiteral("Token stored")) {
                emit obj->loginTokenChanged();
                obj->m_tokenOpInFlight = false;
            }
        },
        Qt::QueuedConnection);
}
#endif

QVariant gx_app_biometrics_store_token_android(const QString& token, const QString& reason, QObject* ctx)
{
#ifdef Q_OS_ANDROID
    QJniObject jReason = QJniObject::fromString(reason);
    QJniObject jToken = QJniObject::fromString(token);

    QJniObject::callStaticMethod<void>(
        "biometrics/GxBiometrics",
        "storeLoginToken",
        "(JLjava/lang/String;Ljava/lang/String;)V",
        ptr(ctx),
        jReason.object<jstring>(),
        jToken.object<jstring>()
    );

    return QVariantMap{
        { "code", 0 },
        { "message", "started" }
    };
#endif

    return {};
}

QVariant gx_app_biometrics_load_token_android(const QString& reason, QObject* ctx)
{
#ifdef Q_OS_ANDROID
    QJniObject jReason = QJniObject::fromString(reason);

    QJniObject::callStaticMethod<void>(
        "biometrics/GxBiometrics",
        "loadLoginToken",
        "(JLjava/lang/String;)V",
        ptr(ctx),
        jReason.object<jstring>()
    );

    return QVariantMap{
        { "code", 0 },
        { "message", "started" },
        { "token", "" }
    };
#endif

    return {};
}

bool gx_app_biometrics_clear_token_android(QObject* /*ctx*/)
{
#ifdef Q_OS_ANDROID
    return QJniObject::callStaticMethod<jboolean>(
        "biometrics/GxBiometrics",
        "clearLoginToken",
        "()Z"
    );
#endif

    return false;
}

bool gx_app_biometrics_has_token_android(QObject* /*ctx*/)
{
#ifdef Q_OS_ANDROID
    return QJniObject::callStaticMethod<jboolean>(
        "biometrics/GxBiometrics",
        "hasLoginToken",
        "()Z"
    );
#endif

    return false;
}

#ifdef Q_OS_ANDROID
extern "C" JNIEXPORT void JNICALL
Java_biometrics_GxBiometrics_notifyQtToken(JNIEnv* env, jclass, jlong objPtr, jint code, jstring message, jstring token)
{
    auto* obj = reinterpret_cast<Biometrics*>(objPtr);
    if (!obj) return;

    const QString qMsg = message ? QString::fromUtf8(env->GetStringUTFChars(message, nullptr)) : QString();

    QString qTok;
    if (token) {
        const char* chars = env->GetStringUTFChars(token, nullptr);
        qTok = QString::fromUtf8(chars);
        env->ReleaseStringUTFChars(token, chars);
    }

    QMetaObject::invokeMethod(obj, [obj, code, qMsg, qTok]() {
        emit obj->authenticated(code, qMsg);
        emit obj->loginTokenReady(code, qMsg, qTok);
    }, Qt::QueuedConnection);
}
#endif
