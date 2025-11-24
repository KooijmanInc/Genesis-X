// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

package com.genesisx.background

import android.app.*
import android.content.*
import android.os.*
import androidx.core.app.NotificationCompat
import android.support.v4.media.session.MediaSessionCompat
import android.support.v4.media.session.PlaybackStateCompat
import android.support.v4.media.MediaMetadataCompat
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

//    companion object {
//        @Volatile private var instance: GXAudioService? = null
//        fun notifyAndroidPlayback(playing: Boolean, positionMs: Long) {
//            instance?.syncPlaybackStateFromQt(playing, positionMs)
//        }
//    }

    private lateinit var mediaSession: MediaSessionCompat

    private fun appName(): String = applicationInfo.loadLabel(packageManager).toString()

    private fun sendCmd(cmd: String) {
        val intent = Intent("$packageName.GX_MEDIA_CMD")
            .setPackage(packageName) // restrict to your app
            .putExtra("cmd", cmd)
        sendBroadcast(intent)
    }

    override fun onCreate() {
        super.onCreate()
        createChannel()

        // 1) Create and activate the media session BEFORE building the notification
        mediaSession = MediaSessionCompat(this, "GXAudioSession").apply {
            setFlags(
                MediaSessionCompat.FLAG_HANDLES_MEDIA_BUTTONS or
                MediaSessionCompat.FLAG_HANDLES_TRANSPORT_CONTROLS
            )

            val launch = packageManager.getLaunchIntentForPackage(packageName)
            val sessionActivity = launch?.let {
                PendingIntent.getActivity(
                    this@GXAudioService, 0, it,
                    PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
                )
            }
            setSessionActivity(sessionActivity)

            val mediaButtonIntent = Intent(Intent.ACTION_MEDIA_BUTTON)
                .setClass(this@GXAudioService, androidx.media.session.MediaButtonReceiver::class.java)
            val mediaButtonPending = PendingIntent.getBroadcast(
                this@GXAudioService, 0, mediaButtonIntent, PendingIntent.FLAG_IMMUTABLE
            )
            setMediaButtonReceiver(mediaButtonPending)

            // Optional callbacks (wire to your Qt layer if you want)
            setCallback(object : MediaSessionCompat.Callback() {
                override fun onPlay() {
                    sendCmd("play")
                    updatePlaybackState(true /*playing*/, /*position*/PlaybackStateCompat.PLAYBACK_POSITION_UNKNOWN)
                    startForeground(NOTIF_ID, buildNotification(false))
                }
                override fun onPause() {
                    sendCmd("pause")
                    updatePlaybackState(false /*paused*/)
                    // Keep foreground while paused (typical for continuous playback apps)
                    startForeground(NOTIF_ID, buildNotification(false))
                }
                override fun onSkipToNext() {
                    sendCmd("next")
                    updatePlaybackState(true /*playing*/, /*position*/PlaybackStateCompat.PLAYBACK_POSITION_UNKNOWN)
                    startForeground(NOTIF_ID, buildNotification(false))
                }
                override fun onSkipToPrevious() {
                    sendCmd("previous")
                }
                override fun onStop() {
                    sendCmd("stop")
                    updatePlaybackState(false)
                    stopForeground(true)
                    stopSelf()
                }
            })
            isActive = true
        }

        // 2) Seed an initial PLAYING state (or PAUSED if you prefer)
        updatePlaybackState(true)

        // 3) Now it's safe to build the notification
        startForeground(NOTIF_ID, buildNotification(true))
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        when (intent?.action) {
            Intent.ACTION_MEDIA_BUTTON -> {
                MediaButtonReceiver.handleIntent(mediaSession, intent)
            }
            ACTION_STOP -> { mediaSession.controller.transportControls.stop() }
            ACTION_PLAY_PAUSE -> {
                val playing = mediaSession.controller.playbackState?.state == PlaybackStateCompat.STATE_PLAYING
                if (playing) mediaSession.controller.transportControls.pause()
                else mediaSession.controller.transportControls.play()
            }
            ACTION_NEXT -> mediaSession.controller.transportControls.skipToNext()
            ACTION_PREVIOUS -> mediaSession.controller.transportControls.skipToPrevious()
            else -> {
                // Let MediaButtonReceiver handle headset/BT buttons:
                MediaButtonReceiver.handleIntent(mediaSession, intent)
            }
        }
        return START_STICKY
    }

    override fun onBind(intent: Intent?): IBinder? = null

    override fun onDestroy() {
        super.onDestroy()
        if (this::mediaSession.isInitialized) {
            mediaSession.isActive = false
            mediaSession.release()
        }
    }

    private var isPlaying = true

    private fun updatePlaybackState(playing: Boolean, positionMs: Long = PlaybackStateCompat.PLAYBACK_POSITION_UNKNOWN) {
        val state = PlaybackStateCompat.Builder()
            .setActions(PlaybackStateCompat.ACTION_PLAY or
                        PlaybackStateCompat.ACTION_PAUSE or
                        PlaybackStateCompat.ACTION_PLAY_PAUSE or
                        PlaybackStateCompat.ACTION_SKIP_TO_NEXT or
                        PlaybackStateCompat.ACTION_SKIP_TO_PREVIOUS or
                        PlaybackStateCompat.ACTION_STOP)
            .setState(
                if (playing) PlaybackStateCompat.STATE_PLAYING else PlaybackStateCompat.STATE_PAUSED,
                positionMs,
                if (playing) 1f else 0f,
                SystemClock.elapsedRealtime()
            )
            .build()
        mediaSession.setPlaybackState(state)
        mediaSession.isActive = true
    }

//    fun setNowPlaying(title: String, artist: String?, album: String?) {
////        val md = android.support.v4.media.MediaMetadataCompat.Builder()
////            .putString(MediaMetadataCompat.METADATA_KEY_TITLE, title)
////            .putString(MediaMetadataCompat.METADATA_KEY_ARTIST, artist)
////            .putString(MediaMetadataCompat.METADATA_KEY_ALBUM, album)
////            .build()
////        mediaSession.setMetadata(md)
//        title: String,
//        artist: String? = null,
//        album: String? = null,
//        durationMs: Long = MediaMetadataCompat.METADATA_KEY_DURATION.toLong(), // pass real duration if you have it
//        albumArt: Bitmap? = null
//    ) {
//        val b = MediaMetadataCompat.Builder()
//            // What SystemUI/BT usually show first:
//            .putString(MediaMetadataCompat.METADATA_KEY_DISPLAY_TITLE, title)
//            .putString(MediaMetadataCompat.METADATA_KEY_TITLE, title)
//            .putString(MediaMetadataCompat.METADATA_KEY_ARTIST, artist)
//            .putString(MediaMetadataCompat.METADATA_KEY_ALBUM, album)

//        if (durationMs > 0) {
//            b.putLong(MediaMetadataCompat.METADATA_KEY_DURATION, durationMs)
//        }
//        if (albumArt != null) {
//            // Both keys are widely consumed by SystemUI/BT
//            b.putBitmap(MediaMetadataCompat.METADATA_KEY_ALBUM_ART, albumArt)
//            b.putBitmap(MediaMetadataCompat.METADATA_KEY_ART, albumArt)
//        }

//        mediaSession.setMetadata(b.build())

//        // Refresh notification so text/art update immediately
//        val playing = mediaSession.controller.playbackState?.state == PlaybackStateCompat.STATE_PLAYING
//        startForeground(NOTIF_ID, buildNotification(playing))
//    }

    private fun createChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val chan = NotificationChannel(
                CHANNEL_ID, "Media Playback", NotificationManager.IMPORTANCE_LOW
            ).apply { lockscreenVisibility = Notification.VISIBILITY_PUBLIC }
            getSystemService(NotificationManager::class.java).createNotificationChannel(chan)
        }
    }

    private fun appDrawable(name: String): Int =
        resources.getIdentifier(name, "drawable", packageName)

    private fun iconOr(name: String, fallback: Int): Int =
        appDrawable(name).takeIf { it != 0 } ?: fallback

    private fun buildNotification(isPlaying: Boolean): Notification {
//        val md = mediaSession.controller.metadata
//        val title  = md?.getString(MediaMetadataCompat.METADATA_KEY_DISPLAY_TITLE)
//            ?: md?.getString(MediaMetadataCompat.METADATA_KEY_TITLE)
//            ?: " "
//        val artist = md?.getString(MediaMetadataCompat.METADATA_KEY_ARTIST)
//        val album  = md?.getString(MediaMetadataCompat.METADATA_KEY_ALBUM)
//        val art    = md?.getBitmap(MediaMetadataCompat.METADATA_KEY_ART)
//            ?: md?.getBitmap(MediaMetadataCompat.METADATA_KEY_ALBUM_ART)

//        val appLabel = applicationInfo.loadLabel(packageManager)?.toString() ?: "App"
//        val subtitle = when {
//            !artist.isNullOrBlank() && !album.isNullOrBlank() -> "$artist • $album"
//            !artist.isNullOrBlank() -> artist
//            else -> appLabel
//        }

        val contentIntent = packageManager.getLaunchIntentForPackage(packageName)?.let {
            PendingIntent.getActivity(this, 0, it,
                PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE)
        }

        val playPausePi = MediaButtonReceiver.buildMediaButtonPendingIntent(
            this,
            if (isPlaying) PlaybackStateCompat.ACTION_PAUSE
            else PlaybackStateCompat.ACTION_PLAY
        )
        val nextPi = MediaButtonReceiver.buildMediaButtonPendingIntent(
            this, PlaybackStateCompat.ACTION_SKIP_TO_NEXT
        )
        val prevPi = MediaButtonReceiver.buildMediaButtonPendingIntent(
            this, PlaybackStateCompat.ACTION_SKIP_TO_PREVIOUS
        )
        val stopIntent = Intent(this, GXAudioService::class.java).apply { action = ACTION_STOP }
        val stopPi = PendingIntent.getService(
            this, 1004, stopIntent, PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )

        val appLabel = applicationInfo.loadLabel(packageManager)?.toString() ?: "App"
        return NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle(appLabel)
            .setContentText("title") // <- your app’s display name
            .setSmallIcon(applicationInfo.icon) // ensure this is a monochrome notif icon on 8.0+
            .setContentIntent(contentIntent)
            .setOngoing(isPlaying)
            .addAction(
                NotificationCompat.Action(
                    android.R.drawable.ic_media_previous, "Previous", prevPi
                )
            )
            .addAction(
                NotificationCompat.Action(
                    if (isPlaying) android.R.drawable.ic_media_pause
                    else android.R.drawable.ic_media_play,
                    if (isPlaying) "Pause" else "Play",
                    playPausePi
                )
            )
            .addAction(
                NotificationCompat.Action(
                    android.R.drawable.ic_media_next, "Next", nextPi
                )
            )
            .addAction(
                NotificationCompat.Action(
                    android.R.drawable.ic_menu_close_clear_cancel, "Stop", stopPi
                )
            )
            .setStyle(
                androidx.media.app.NotificationCompat.MediaStyle()
                    .setMediaSession(mediaSession.sessionToken)
                    .setShowActionsInCompactView(0, 1, 2) // prev, play/pause, next
                    .setShowCancelButton(false)
            )
            .setVisibility(NotificationCompat.VISIBILITY_PUBLIC)
            .build()
    }

    private fun promoteWithCurrentState() {
        val st = mediaSession.controller.playbackState?.state
        val playing = st == PlaybackStateCompat.STATE_PLAYING || st == PlaybackStateCompat.STATE_BUFFERING
        startForeground(NOTIF_ID, buildNotification(playing))
    }

    private fun syncPlaybackStateFromQt(isPlaying: Boolean, positionMs: Long) {
        val state = PlaybackStateCompat.Builder()
            .setActions(
                PlaybackStateCompat.ACTION_PLAY or
                PlaybackStateCompat.ACTION_PAUSE or
                PlaybackStateCompat.ACTION_PLAY_PAUSE or
                PlaybackStateCompat.ACTION_SKIP_TO_NEXT or
                PlaybackStateCompat.ACTION_SKIP_TO_PREVIOUS or
                PlaybackStateCompat.ACTION_STOP
            )
            .setState(
                if (isPlaying) PlaybackStateCompat.STATE_PLAYING else PlaybackStateCompat.STATE_PAUSED,
                positionMs,
                if (isPlaying) 1f else 0f,
                android.os.SystemClock.elapsedRealtime()
            )
            .build()

        mediaSession.setPlaybackState(state)
        mediaSession.isActive = true
        // keep the notif in sync with the REAL state:
        startForeground(NOTIF_ID, buildNotification(isPlaying))
    }
}
