// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXWEBSITE_H
#define GXWEBSITE_H

#include <QQuickItem>
#include <QUrl>

#include "genesisx_web_global.h"
#ifdef Q_OS_WIN
#include "src/windows/GXWebView2Host.h"
#endif

namespace gx::web {

class GENESISX_WEB_EXPORT GXWebsite : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QUrl currentUrl READ currentUrl NOTIFY currentUrlChanged)
    Q_PROPERTY(bool devToolsWindow READ devToolsWindow WRITE setDevToolsWindow NOTIFY devToolsWindowChanged)

public:
    explicit GXWebsite(QQuickItem* parent = nullptr);

    QUrl url() const { return m_url; }
    void setUrl(const QUrl& u);

    QUrl currentUrl() const { return m_currentUrl; }

    bool devToolsWindow() const { return m_devToolsWindow; }
    void setDevToolsWindow(bool d);

signals:
    void urlChanged();
    void currentUrlChanged();
    void devToolsWindowChanged();

protected:
    void componentComplete() override;
    void releaseResources() override;
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

private:
    void ensureNativeHost();
    void destroyNativeHost();
    void updateNativeHostGeometry();

    QUrl m_url;
    QUrl m_currentUrl;
    bool m_devToolsWindow = false;

#if defined(Q_OS_WIN)
    void* m_hwndHost = nullptr;
    std::unique_ptr<GXWebView2Host> m_webView2;
#endif
};

}

#endif // GXWEBSITE_H
