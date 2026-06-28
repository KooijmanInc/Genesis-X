// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <QQuickWindow>

#if defined(Q_OS_WIN)
#include <windows.h>
#include <WebView2.h>
#endif

#include <GenesisX/Web/GXWebsite.h>
// #include "../src/windows/GXWebView2Host.h"
// #include "../src/windows/GXWebView2Loader.h"

namespace gx::web {

GXWebsite::GXWebsite(QQuickItem *parent)
    : QQuickItem{parent}
{
    setFlag(ItemHasContents, false);
}

void GXWebsite::setUrl(const QUrl& u)
{
    if (m_url == u) return;
    m_url = u;

    emit urlChanged();

    if (m_currentUrl != m_url) {
        m_currentUrl = m_url;
        emit currentUrlChanged();
    }

#ifdef Q_OS_WIN
    m_webView2->setUrl(m_url);
    if (m_webView2 && m_webView2->isReady()) {
        m_webView2->navigate(m_url.toString());
    }
#endif
}

void GXWebsite::setDevToolsWindow(bool d)
{
    if (m_devToolsWindow == d) return;
    m_devToolsWindow = d;

    emit devToolsWindowChanged();
#ifdef Q_OS_WIN
    m_webView2->setDevToolsWindow(m_devToolsWindow);
#endif
}

void GXWebsite::componentComplete()
{
    QQuickItem::componentComplete();
    ensureNativeHost();
    updateNativeHostGeometry();
}

void GXWebsite::releaseResources()
{
    destroyNativeHost();
    QQuickItem::releaseResources();
}

void GXWebsite::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    updateNativeHostGeometry();
}

void GXWebsite::ensureNativeHost()
{
#if defined(Q_OS_WIN)
    if (m_hwndHost) return;

    if (!window()) return;

    HWND parentHwnd = reinterpret_cast<HWND>(window()->winId());
    if (!parentHwnd) return;

    HWND host = CreateWindowExW(
        0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE,
        0, 0, 10, 10,
        parentHwnd, nullptr, GetModuleHandleW(nullptr), nullptr
    );

    m_hwndHost = host;

    if (!m_webView2) {
        m_webView2 = std::make_unique<GXWebView2Host>(reinterpret_cast<HWND>(m_hwndHost));
        connect(m_webView2.get(), &GXWebView2Host::currentUrlChanged, this, [this](const QString& u) {
            m_currentUrl = QUrl(u);
            emit currentUrlChanged();
        });
        m_webView2->initialize();
        if (m_url.isValid()) m_webView2->navigate(m_url.toString());
    }
#endif
}

void GXWebsite::destroyNativeHost()
{
#if defined(Q_OS_WIN)
    if (!m_hwndHost) return;
    DestroyWindow(reinterpret_cast<HWND>(m_hwndHost));
    m_hwndHost = nullptr;
#endif
}

void GXWebsite::updateNativeHostGeometry()
{
#if defined(Q_OS_WIN)
    if (!m_hwndHost || !window()) return;

    QPointF topLeft = mapToScene(QPointF(0, 0));
    QRectF r(topLeft, QSizeF(width(), height()));

    const qreal dpr = window()->devicePixelRatio();

    const int x = int(r.x() * dpr);
    const int y = int(r.y() * dpr);
    const int w = int(r.width() * dpr);
    const int h = int(r.height() * dpr);

    SetWindowPos(reinterpret_cast<HWND>(m_hwndHost), nullptr, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);

    if (m_webView2) {
        m_webView2->setBounds(QRect(0, 0, w, h));
    }
#endif
}

}
