// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "fcm_android.h"

#ifdef Q_OS_ANDROID

#include <QCoreApplication>
#include <QtCore/qnativeinterface.h>

using namespace gx::android;

void FcmListener::OnTokenReceived(const char *token)
{
    if (!m_owner) return;
    m_owner->m_token = QString::fromUtf8(token ? token : "[GX Notify] token failed");
    emit m_owner->tokenChanged(m_owner->m_token);
}

static QVariantMap mapFromPairs(const ::firebase::messaging::Message& msg)
{
    QVariantMap out;
#if FIREBASE_VERSION_MAJOR >= 11
    // Newer SDKs expose msg.data as std::map<std::string,std::string>
    for (const auto& kv : msg.data) {
        out.insert(QString::fromStdString(kv.first),
                   QString::fromStdString(kv.second));
    }
#else
    // Fallback – some older SDKs exposed raw_data. Keep both just in case.
    if (!msg.raw_data.empty()) {
        qDebug() << msg.raw_data;
        // out["raw"] = QString::fromUtf8(reinterpret_cast<const char*>(msg.raw_data),
                                       // int(msg.raw_data_size));
    }
#endif
    return out;
}

void FcmListener::OnMessage(const ::firebase::messaging::Message &message)
{
    QString title, body;
    if (message.notification) {
        if (message.notification->title != "") title = QString::fromUtf8(message.notification->title);
        if (message.notification->body != "")  body  = QString::fromUtf8(message.notification->body);
    }
    if (title.isEmpty()) title = QStringLiteral("FCM");
    const QVariantMap data = mapFromPairs(message);

    QMetaObject::invokeMethod(&FcmBridge::instance(), [title, body, data]{
        emit FcmBridge::instance().messageReceived(title, body, data);
    }, Qt::QueuedConnection);
}

void FcmBridge::initialize()
{
    ::firebase::App* inst = ::firebase::App::GetInstance();
    if (!inst) {
        QJniEnvironment env;
        auto ctx = QNativeInterface::QAndroidApplication::context();
        jobject jctx = ctx.object();
        inst = ::firebase::App::Create(env.jniEnv(), jctx);
    }
    m_app = inst;

    if (!m_listener)
        m_listener = std::make_unique<FcmListener>(this);

    m_initializer.Initialize(m_app, this, &FcmBridge::initThunk);
}

firebase::InitResult FcmBridge::initThunk(firebase::App *app, void *ctx)
{
    auto* self = static_cast<FcmBridge*>(ctx);
    return ::firebase::messaging::Initialize(*app, self->listener());
}

#endif

