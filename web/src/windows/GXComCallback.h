// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef GXCOMCALLBACK_H
#define GXCOMCALLBACK_H

#include <GenesisX/Web/genesisx_web_global.h>

#ifdef Q_OS_WIN

#include <windows.h>
#include <unknwn.h>
#include <atomic>

namespace gx::web {

class GENESISX_WEB_EXPORT ComRefCounted
{
public:
    ULONG STDMETHODCALLTYPE AddRef() noexcept { return ++m_ref; }
    ULONG STDMETHODCALLTYPE Release() noexcept {
        ULONG r = --m_ref;
        if (r == 0) delete this;
        return r;
    }

protected:
    ComRefCounted() = default;
    virtual ~ComRefCounted() = default;

private:
    std::atomic<ULONG> m_ref{1};
};

}

#endif

#endif // GXCOMCALLBACK_H
