// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXWEBVIEW2LOADER_H
#define GXWEBVIEW2LOADER_H

#include <GenesisX/Web/genesisx_web_global.h>

#if defined(Q_OS_WIN)

#include <windows.h>

namespace gx::web {

class GENESISX_WEB_EXPORT GXWebView2Loader
{
public:
    GXWebView2Loader();
    ~GXWebView2Loader();

    bool isLoaded() const { return m_createEnvironment != nullptr; }

    using CreateEnvFunc = HRESULT (WINAPI*)(PCWSTR, PCWSTR, void*, void*);

    CreateEnvFunc createEnvironment() const { return m_createEnvironment; }

private:
    HMODULE m_module = nullptr;
    CreateEnvFunc m_createEnvironment = nullptr;
};

}

#endif

#endif // GXWEBVIEW2LOADER_H
