// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef FCM_ANDROID_H
#define FCM_ANDROID_H

#include <QObject>
#include <QVariantMap>

#include <GenesisX/genesisx_global.h>

#ifdef Q_OS_ANDROID
#include <memory>
#include <firebase/app.h>
#include <firebase/messaging.h>
#include <firebase/util.h>
#endif

namespace gx::android {

#ifdef Q_OS_ANDROID

class FcmBridge;

class GENESISX_CORE_EXPORT FcmListener final : public ::firebase::messaging::Listener
{
public:
    explicit FcmListener(FcmBridge* owner) : m_owner(owner) {}
    void OnTokenReceived(const char* token) override;
    void OnMessage(const ::firebase::messaging::Message& message) override;

private:
    FcmBridge* m_owner = nullptr;
};

class GENESISX_CORE_EXPORT FcmBridge : public QObject
{
    Q_OBJECT

public:
    static FcmBridge& instance() {
        static FcmBridge s;
        return s;
    }

    void initialize();
    QString token() const { return m_token; }

    void subscribe(const QString& topic);
    void unsubscribe(const QString& topic);

    FcmListener* listener() { return m_listener.get(); }
    const FcmListener* listener() const { return m_listener.get(); }

    QString m_token;

signals:
    void tokenChanged(const QString& token);
    void messageReceived(const QString& title, const QString& body, const QVariantMap& data);

private:
    FcmBridge() = default;
    Q_DISABLE_COPY_MOVE(FcmBridge)

    static ::firebase::InitResult initThunk(::firebase::App* app, void* ctx);

    ::firebase::App* m_app = nullptr;
    ::firebase::ModuleInitializer m_initializer;
    std::unique_ptr<FcmListener> m_listener;
};
#else
class GENESISX_CORE_EXPORT FcmBridge : public QObject
{
    Q_OBJECT
public:
    static FcmBridge& instance() {
        static FcmBridge s;
        return s;
    }
    void initialize() {}
    QString token() const { return {}; }
    void subscribe(const QString&) {}
    void unsubscribe(const QString&) {}

signals:
    void tokenChanged(const QString& token);
    void messageReceived(const QString& title, const QString& body, const QVariantMap& data);

private:
    FcmBridge() = default;
    Q_DISABLE_COPY_MOVE(FcmBridge)
};

// class GENESISX_CORE_EXPORT FcmLFcmListener {};
#endif

}

#endif // FCM_ANDROID_H

