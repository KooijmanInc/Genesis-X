// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

package com.genesisx.background

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.util.Log

class GXMediaCmdReceiver : BroadcastReceiver() {
    override fun onReceive(ctx: Context, intent: Intent) {
        if (intent.action != "${ctx.packageName}.GX_MEDIA_CMD") return
        val cmd = intent.getStringExtra("cmd") ?: return
        val pr = goAsync()
        Thread {
            try { BackgroundBridge.dispatchCommand(cmd) }
            finally { pr.finish() }
        }.start()
    }
}
