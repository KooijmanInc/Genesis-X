// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

package com.genesisx.background

object BackgroundBridge {
    init {
//        System.loadLibrary("hm-ui");
        System.loadLibrary("genesisx_arm64-v8a");
    }
    @JvmStatic external fun dispatchCommand(cmd: String)
}
