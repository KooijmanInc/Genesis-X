package com.genesisx.background

import android.app.*
import android.content.*
import android.os.*
import androidx.core.app.NotificationCompat
import android.support.v4.media.session.MediaSessionCompat
import android.support.v4.media.session.PlaybackStateCompat
import androidx.media.app.NotificationCompat.MediaStyle
import androidx.media.session.MediaButtonReceiver

class GXAudioService : Service() {

    companion object {
        const val CHANNEL_ID = "gx_audio_playback"
        const val NOTIF_ID = 1001
        const val ACTION_STOP = "com.genesisx.background.ACTION_STOP"
        const val ACTION_PLAY_PAUSE = "com.genesisx.background.ACTION_PLAY_PAUSE"
        const val ACTION_NEXT = "com.genesisx.background.ACTION_NEXT"
        const val ACTION_PREVIOUS = "com.genesisx.background.ACTION_PREVIOUS"
    }

    private lateinit var mediaSession: MediaSessionCompat

    override fun onCreate() {
        super.onCreate()
        createChannel()

        mediaSession = MediaSessionCompat(this, "GXAudioSession").apply {
            setFlags(
                MediaSessionCompat.FLAG_HANDLES_MEDIA_BUTTONS or
                MediaSessionCompat.FLAGS_HANDLES_TRANSPORT_CONTROLS
            )
            setCallback(object : MediaSessionCompat.Callback() {
                override fun onPlay() {
                    updatePlaybackState(isPlaying = true)
                    startForeground(NOTIF_ID, buildNotification(isPlaying = true))
                }
                override fun onPause() {
                    updatePlaybackState(isPlaying = false)
                    startForeground(NOTIF_ID, buildNotification(isPlaying = false))
                }
                override fun onSkipToNext() {
                    // TODO: signal qt to advance
                }
                override fun onSkipToPrevious() {
                    // TODO: signal qt to previous
                }
                override fun onStop() {
                    stopForeground(true)
                    stopSelf()
                }
            })
            isActive = true
        }

        updatePlaybackState(isPlayling = true)

        startForeground(NOTIF_ID, buildNotification(isPlaying = true))
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        when (intent?.action) {
            ACTION_STOP -> { stopForeground(true); stopSelf() }
            ACTION_PLAY_PAUSE -> {
                startForeground(NOTIF_ID, buildNotification(isPlaying = true))
            }
        }
        return START_STICKY
    }

    override fun onBind(intent: Intent?): IBinder? = null

    private fun createChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val chan = NotificationChannel(
                CHANNEL_ID, "Media Playback", NotificationManager.IMPORTANCE_LOW
            )
            chan.lockscreenVisibility = Notification.VISIBILITY_PUBLIC
            getSystemService(NotificationManager::class.java).createNotificationChannel(chan)
        }
    }

    private fun buildNotification(isPlaying: Boolean): Notification {
        val ctx = this

        val stopIntent = Intent(this, GXAudioService::class.java).apply { action = ACTION_STOP }
        val stopPending = PendingIntent.getService(
            this, 1, stopIntent,
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )
        val contentIntent = packageManager.getLaunchIntentForPackage(packageName)?.let {
            PendingIntent.getActivity(this, 0, it,
                PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE)
        }

        return NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("Playing audio")
            .setContentText("Genesis-X")
            .setSmallIcon(applicationInfo.icon)
            .setContentIntent(contentIntent)
            .setOngoing(isPlaying)
            // (no MediaStyle yet)
            .setStyle(androidx.media.app.NotificationCompat.MediaStyle()
                .setMediaSession(mediaSession.sessionToken)
                .setShowActionsInCompactView(0)
                )
            .addAction(android.R.drawable.ic_menu_close_clear_cancel, "Stop", stopPending)
            .build()
    }
}


//package com.genesisx.background

//import android.app.*
//import android.content.*
//import android.os.*
//import androidx.core.app.NotificationCompat
//import androidx.media.session.MediaSessionCompat
//import androidx.media.app.NotificationCompat.MediaStyle

//class GXAudioService : Service() {

//    companion object {
//        const val CHANNEL_ID = "gx_audio_playback"
//        const val NOTIF_ID = 1001
//        const val ACTION_STOP = "com.genesisx.background.ACTION_STOP"
//        const val ACTION_PLAY_PAUSE = "com.genesisx.background.ACTION_PLAY_PAUSE"
//    }

//    private lateinit var mediaSession: MediaSessionCompat

//    override fun onCreate() {
//        super.onCreate()
//        mediaSession = MediaSessionCompat(this, "GXAudioSession")
//        createChannel()
//        startForeground(NOTIF_ID, buildNotification(isPlaying = true))
//    }

//    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
//        when (intent?.action) {
//            ACTION_STOP -> {
//                stopForeground(true)
//                stopSelf()
//            }
//            ACTION_PLAY_PAUSE -> {
//                // Optionally: broadcast back to Qt to toggle play/pause
//                // Keep it simple for v1: just keep service alive
//                startForeground(NOTIF_ID, buildNotification(isPlaying = true))
//            }
//        }
//        return START_STICKY
//    }

//    override fun onBind(intent: Intent?) = null

//    private fun createChannel() {
//        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
//            val chan = NotificationChannel(
//                CHANNEL_ID, "Media Playback", NotificationManager.IMPORTANCE_LOW
//            )
//            chan.lockscreenVisibility = Notification.VISIBILITY_PUBLIC
//            val nm = getSystemService(NotificationManager::class.java)
//            nm.createNotificationChannel(chan)
//        }
//    }

//    private fun buildNotification(isPlaying: Boolean): Notification {
//        val ctx = this

//        val stopIntent = Intent(ctx, GXAudioService::class.java).apply { action = ACTION_STOP }
//        val stopPending = PendingIntent.getService(
//            ctx, 1, stopIntent,
//            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
//        )

//        val contentIntent = ctx.packageManager.getLaunchIntentForPackage(ctx.packageName)?.let {
//            PendingIntent.getActivity(ctx, 0, it,
//                PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE)
//        }

//        val builder = NotificationCompat.Builder(ctx, CHANNEL_ID)
//            .setContentTitle("Playing audio")
//            .setContentText("Genesis-X")
//            .setSmallIcon(ctx.applicationInfo.icon)
//            .setContentIntent(contentIntent)
//            .setOngoing(isPlaying)
//            .setStyle(MediaStyle().setMediaSession(mediaSession.sessionToken))
//            .addAction(android.R.drawable.ic_menu_close_clear_cancel, "Stop", stopPending)

//        return builder.build()
//    }
//}
