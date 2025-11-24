// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

package com.genesisx.background

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.util.Log

class GXMediaCmdReceiver : BroadcastReceiver() {
    override fun onReceive(ctx: Context, intent: Intent) {
        val expected = "${ctx.packageName}.GX_MEDIA_CMD"
        val action = intent.action ?: expected
        if (action != expected) return

        val cmd = intent.getStringExtra("cmd") ?: return

        return;
        // 1) ensure the app’s native library is loaded when we’re woken by AlarmManager
        try {
            // use your actual app lib name (the one that produces lib<name>.so)
            System.loadLibrary("genesisx")   // <-- CHANGE if your .so base name differs
        } catch (e: UnsatisfiedLinkError) {
            android.util.Log.e("GXMediaCmdReceiver", "loadLibrary failed", e)
            return
        }

        // 2) optional: start the keep-alive service so Doze won’t kill us mid-write
        val svc = Intent(ctx, com.genesisx.cast.GXCastKeepAliveService::class.java)
        if (android.os.Build.VERSION.SDK_INT >= 26) ctx.startForegroundService(svc) else ctx.startService(svc)

        // 3) now it’s safe to call JNI
        val pr = goAsync()
        Thread {
            try { BackgroundBridge.dispatchCommand(cmd) }
            finally { pr.finish() }
        }.start()
    }

//    override fun onReceive(ctx: Context, intent: Intent) {
//        // Accept our custom action; default to it if alarm didn't set one
//        val expected = "${ctx.packageName}.GX_MEDIA_CMD"
//        val action = intent.action ?: expected
//        if (action != expected) return

//        val cmd = intent.getStringExtra("cmd") ?: return
//        val pr = goAsync()

//        when (cmd) {
//            "cast_check_end" -> {
//                // Make sure the keep-alive service is up so we can save state under doze
//                val svc = Intent(ctx, com.genesisx.cast.GXCastKeepAliveService::class.java)
//                if (android.os.Build.VERSION.SDK_INT >= 26) ctx.startForegroundService(svc) else ctx.startService(svc)

//                // Now bounce into your Qt/bridge to query RMC & persist last-played
//                Thread {
//                    try {
//                        BackgroundBridge.dispatchCommand("cast_check_end")
//                    } finally {
//                        pr.finish()
//                    }
//                }.start()
//            }
//            else -> {
//                Thread {
//                    try {
//                        BackgroundBridge.dispatchCommand(cmd)
//                    } finally {
//                        pr.finish()
//                    }
//                }.start()
//            }
//        }
//    }

//    override fun onReceive(ctx: Context, intent: Intent) {
//        if (intent.action != "${ctx.packageName}.GX_MEDIA_CMD") return
//        val cmd = intent.getStringExtra("cmd") ?: return
//        val pr = goAsync()
//        Thread {
//            try { BackgroundBridge.dispatchCommand(cmd) }
//            finally { pr.finish() }
//        }.start()
//    }
}
