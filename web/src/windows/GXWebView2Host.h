// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXWEBVIEW2HOST_H
#define GXWEBVIEW2HOST_H

#include <GenesisX/Web/genesisx_web_global.h>

#if defined(Q_OS_WIN)

#include <QObject>
#include <QString>
#include <QRect>
#include <QUrl>

#include <wrl.h>
#include <windows.h>
#include <WebView2.h>

namespace gx::web {

class GENESISX_WEB_EXPORT GXWebView2Host : public QObject
{
    Q_OBJECT

public:
    explicit GXWebView2Host(HWND hostHwnd, QObject *parent = nullptr);
    ~GXWebView2Host() override;

    bool initialize();
    void setUrl(const QUrl& url);
    void setDevToolsWindow(bool d);

    bool isReady() const { return m_webView != nullptr; }

    void setBounds(const QRect& pxRect);
    void navigate(const QString& url);

signals:
    void readyChanged(bool ready);
    void currentUrlChanged(const QString& url);
    void loadFinished(bool ok, const QString& errorString);

private:
    void applyPendingUrl();

    friend class CreateEnvironmentCompletedHandler;
    friend class CreateControllerCompletedHandler;
    friend class SourceChangedHandler;

    void onEnvironmentCreated(HRESULT result, ICoreWebView2Environment* env);
    void onControllerCreated(HRESULT result, ICoreWebView2Controller* controller);
    void onSourceChanged(ICoreWebView2* sender);

    bool ensureComInitialized();

    HWND m_hostHwnd = nullptr;

    Microsoft::WRL::ComPtr<ICoreWebView2Environment> m_env;
    Microsoft::WRL::ComPtr<ICoreWebView2Controller> m_controller;
    Microsoft::WRL::ComPtr<ICoreWebView2> m_webView;

    bool m_comOk = false;
    bool m_ready = false;
    QUrl m_pendingUrl;
    bool m_devToolsWindow = false;
};

}

#endif

#endif // GXWEBVIEW2HOST_H
