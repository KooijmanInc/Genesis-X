// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

package com.genesisx.cast;

import android.os.*;
import android.app.*;
import android.content.*;
import androidx.core.app.NotificationCompat;
import android.content.pm.ServiceInfo;

public class GXCastKeepAliveService extends Service {
    public static final String CHANNEL_ID = "gx_cast";
    public static final int NOTIF_ID = 1001;

    private PowerManager.WakeLock wake;

    @Override public void onCreate() {
        super.onCreate();
        NotificationManager nm = getSystemService(NotificationManager.class);
        NotificationChannel ch = new NotificationChannel(
                CHANNEL_ID, "Casting", NotificationManager.IMPORTANCE_LOW);
        nm.createNotificationChannel(ch);

        Notification notif = new NotificationCompat.Builder(this, CHANNEL_ID)
                .setContentTitle("Casting in progress")
                .setContentText("Your playlist will stay in sync.")
                .setSmallIcon(android.R.drawable.ic_media_play)
                .setOngoing(true)
                .build();

        // FOREGROUND_SERVICE_TYPE_MEDIA_PLAYBACK is key on 12+
        startForeground(NOTIF_ID, notif, ServiceInfo.FOREGROUND_SERVICE_TYPE_MEDIA_PLAYBACK);
    }

    /** Acquire a short PARTIAL wakelock window for end-of-track saves. */
    public void pulseWakeLock(long millis) {
        PowerManager pm = (PowerManager)getSystemService(POWER_SERVICE);
        if (wake == null) {
            wake = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "gx:cast");
            wake.setReferenceCounted(false);
        }
        if (!wake.isHeld()) wake.acquire(millis);
    }

    @Override public int onStartCommand(Intent i, int flags, int id) {
        return START_STICKY;
    }

    @Override public void onDestroy() {
        if (wake != null && wake.isHeld()) wake.release();
        super.onDestroy();
    }

    @Override public IBinder onBind(Intent intent) { return null; }
}
