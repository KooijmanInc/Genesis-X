// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "GXComCallback.h"
#include "GXWebView2Host.h"
#include "GXWebView2Loader.h"

#if defined(Q_OS_WIN)

#include <QStandardPaths>
#include <QDebug>
#include <QUrl>
#include <QDir>

#include <objbase.h>

using Microsoft::WRL::ComPtr;

namespace gx::web {

static std::wstring toWide(const QString& s)
{
    return std::wstring(reinterpret_cast<const wchar_t*>(s.utf16()));
}

class GXWebView2Host;

class SourceChangedHandler final
    : public ICoreWebView2SourceChangedEventHandler
    , public ComRefCounted
{
public:
    explicit SourceChangedHandler(GXWebView2Host* host) : m_host{host} {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
    {
        if (!ppvObject) return E_POINTER;
        *ppvObject = nullptr;


        if (IsEqualGUID(riid, IID_IUnknown) || IsEqualGUID(riid, IID_ICoreWebView2SourceChangedEventHandler)) {
            *ppvObject = static_cast<ICoreWebView2SourceChangedEventHandler*>(this);
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override { return ComRefCounted::AddRef(); }
    ULONG STDMETHODCALLTYPE Release() override { return ComRefCounted::Release(); }

    HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2* sender, ICoreWebView2SourceChangedEventArgs*) override;

private:
    GXWebView2Host* m_host = nullptr;
};

class CreateControllerCompletedHandler final
    : public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler
    , public ComRefCounted
{
public:
    explicit CreateControllerCompletedHandler(GXWebView2Host* host) : m_host(host) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
    {
        if (!ppvObject) return E_POINTER;
        *ppvObject = nullptr;

        if (IsEqualGUID(riid, IID_IUnknown) || IsEqualGUID(riid, IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler)) {
            *ppvObject = static_cast<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override { return ComRefCounted::AddRef(); }
    ULONG STDMETHODCALLTYPE Release() override { return ComRefCounted::Release(); }

    HRESULT STDMETHODCALLTYPE Invoke(HRESULT result, ICoreWebView2Controller* controller) override;

private:
    GXWebView2Host* m_host = nullptr;
};

class CreateEnvironmentCompletedHandler final
    : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler
    , public ComRefCounted
{
public:
    explicit CreateEnvironmentCompletedHandler(GXWebView2Host* host) : m_host(host) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
    {
        if (!ppvObject) return E_POINTER;
        *ppvObject = nullptr;

        if (IsEqualGUID(riid, IID_IUnknown) || IsEqualGUID(riid, IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler)) {
            *ppvObject = static_cast<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override { return ComRefCounted::AddRef(); }
    ULONG STDMETHODCALLTYPE Release() override { return ComRefCounted::Release(); }

    HRESULT STDMETHODCALLTYPE Invoke(HRESULT result, ICoreWebView2Environment* env) override;

private:
    GXWebView2Host* m_host = nullptr;
};

HRESULT SourceChangedHandler::Invoke(ICoreWebView2* sender, ICoreWebView2SourceChangedEventArgs*)
{
    if (m_host) m_host->onSourceChanged(sender);
    return S_OK;
}

HRESULT CreateControllerCompletedHandler::Invoke(HRESULT result, ICoreWebView2Controller* controller)
{
    if (m_host) m_host->onControllerCreated(result, controller);
    return S_OK;
}

HRESULT CreateEnvironmentCompletedHandler::Invoke(HRESULT result, ICoreWebView2Environment* env)
{
    if (m_host) m_host->onEnvironmentCreated(result, env);
    return S_OK;
}

GXWebView2Host::GXWebView2Host(HWND hostHwnd, QObject *parent)
    : QObject{parent}
    , m_hostHwnd{hostHwnd}
{}

GXWebView2Host::~GXWebView2Host()
{
    if (m_controller) m_controller->Close();

    m_webView.Reset();
    m_controller.Reset();
    m_env.Reset();
}

bool GXWebView2Host::initialize()
{
    if (!m_hostHwnd) {
        return false;
    }
    if (!ensureComInitialized()) {
        return false;
    }
    if (m_webView) {

        return true;
    }

    GXWebView2Loader loader;
    if (!loader.isLoaded()) {
        qWarning() << "[GXWeb] .dll not found";
        return false;
    }

    auto createEnv = loader.createEnvironment();

    auto* handler = new CreateEnvironmentCompletedHandler(this);

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);

    QString webViewDir = QDir(dataDir).absoluteFilePath("WebView2");
    QDir().mkpath(webViewDir);

    std::wstring wUserData = std::wstring(reinterpret_cast<const wchar_t*>(webViewDir.utf16()));

    HRESULT hr = createEnv(nullptr, wUserData.c_str(), nullptr, handler);

    if (FAILED(hr)) {
        qWarning() << "WebView2: CreateCoreWebView2EnvironmentWithOptions failed:" << Qt::hex << hr;
        return false;
    }

    return true;
}

void GXWebView2Host::setUrl(const QUrl &url)
{
    m_pendingUrl = url;

    if (!m_ready || !m_webView) {
        qDebug() << "setUrl queued (not ready yet):" << url;
        return;
    }

    applyPendingUrl();
}

void GXWebView2Host::setDevToolsWindow(bool d)
{
    if (m_devToolsWindow == d) return;
    m_devToolsWindow = d;
}

void GXWebView2Host::setBounds(const QRect &pxRect)
{
    if (!m_controller)
        return;

    RECT r;
    r.left = pxRect.left();
    r.top = pxRect.top();
    r.right = pxRect.left() + pxRect.width();
    r.bottom = pxRect.top() + pxRect.height();

    m_controller->put_Bounds(r);
}

void GXWebView2Host::navigate(const QString &url)
{
    if (!m_webView)
        return;

    // Normalize URL (avoid empty)
    const QString finalUrl = url.isEmpty() ? QStringLiteral("about:blank") : url;

    std::wstring w = toWide(finalUrl);
    m_webView->Navigate(w.c_str());
}

void GXWebView2Host::applyPendingUrl()
{
    if (!m_webView || !m_pendingUrl.isValid())
        return;

    const std::wstring w = m_pendingUrl.toString().toStdWString();
    m_webView->Navigate(w.c_str());

    if (m_devToolsWindow) m_webView->OpenDevToolsWindow();
}

void GXWebView2Host::onEnvironmentCreated(HRESULT result, ICoreWebView2Environment *env)
{
    // qDebug() << "onEnvironmentCreated hr=" << Qt::hex << result << "env=" << (void*)env;

    if (FAILED(result) || !env) {
        emit loadFinished(false, QStringLiteral("Failed to create WebView2 environment"));
        return;
    }
    m_env = env;

    // create controller
    auto* handler = new CreateControllerCompletedHandler(this);
    m_env->CreateCoreWebView2Controller(m_hostHwnd, handler);
}

void GXWebView2Host::onControllerCreated(HRESULT result, ICoreWebView2Controller *controller)
{
    // qDebug() << "onControllerCreated hr=" << Qt::hex << result << "controller=" << (void*)controller;

    if (FAILED(result) || !controller) {
        emit loadFinished(false, QStringLiteral("Failed to create WebView2 controller"));
        return;
    }

    m_controller = controller;
    m_controller->get_CoreWebView2(&m_webView);

    if (!m_webView) {
        emit loadFinished(false, QStringLiteral("Failed to get CoreWebView2"));
        return;
    }

    // hook source changed
    EventRegistrationToken token{};
    auto* handler = new SourceChangedHandler(this);
    m_webView->add_SourceChanged(handler, &token);

    emit loadFinished(true, QString());

    RECT bounds{};
    GetClientRect(m_hostHwnd, &bounds);
    m_controller->put_Bounds(bounds);
    m_controller->put_IsVisible(TRUE);

    m_ready = true;
    emit readyChanged(true);

    applyPendingUrl();
}

void GXWebView2Host::onSourceChanged(ICoreWebView2 *sender)
{
    LPWSTR src = nullptr;
    if (sender && SUCCEEDED(sender->get_Source(&src)) && src) {
        QString qsrc = QString::fromWCharArray(src);
        CoTaskMemFree(src);
        emit currentUrlChanged(qsrc);
    }
}

bool GXWebView2Host::ensureComInitialized()
{
    if (m_comOk) return true;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE) {
        m_comOk = true;
        return true;
    }

    qWarning() << "WebView2: CoInitializeEx failed:" << Qt::hex << hr;
    return false;
}

}

#endif
