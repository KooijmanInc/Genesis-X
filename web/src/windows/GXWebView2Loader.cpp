// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "GXWebView2Loader.h"

#if defined(Q_OS_WIN)

using namespace gx::web;

GXWebView2Loader::GXWebView2Loader()
{
    m_module = LoadLibraryW(L"WebView2Loader.dll");
    if (!m_module) return;

    m_createEnvironment = reinterpret_cast<CreateEnvFunc>(GetProcAddress(m_module, "CreateCoreWebView2EnvironmentWithOptions"));
}

GXWebView2Loader::~GXWebView2Loader()
{
    if (m_module) FreeLibrary(m_module);
}

#endif
