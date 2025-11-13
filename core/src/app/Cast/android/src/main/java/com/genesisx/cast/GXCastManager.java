package com.genesisx.cast;

import android.app.Activity;
import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.Nullable;

import com.google.android.gms.cast.MediaInfo;
import com.google.android.gms.cast.MediaMetadata;
import com.google.android.gms.cast.MediaStatus;
import com.google.android.gms.cast.MediaQueueItem;
import com.google.android.gms.cast.framework.CastContext;
import com.google.android.gms.cast.framework.CastSession;
import com.google.android.gms.cast.framework.SessionManager;
import com.google.android.gms.cast.framework.SessionManagerListener;
import com.google.android.gms.cast.framework.media.RemoteMediaClient;

public final class GXCastManager {
    private static final String TAG = "GXCastManager";

    // --- Cast plumbing ---
    private static SessionManager sessionManager;
    private static CastSession currentSession;
    private static CastContext castContext;

    // --- App context for services/alarms ---
    private static Context appContext;

    // --- Status callback (single, static) ---
    private static RemoteMediaClient.Callback statusCb;
    private static boolean statusCbAttached = false;

    // --- Debounce / bookkeeping ---
    private static String lastFinishedContentId = null;
    private static String currentPlayingContentId = null;
    private static Integer lastItemId = null;   // current queue item id
    private static int lastPlayerState = MediaStatus.PLAYER_STATE_UNKNOWN;

    private static boolean listenerRegistered = false;

    private static final int REQ_CODE_END_PING = 42;

    private GXCastManager() {}

    // -------------------- Init / lifecycle --------------------

    public static void init(Context ctx) {
        try {
            if (castContext == null) {
                castContext = CastContext.getSharedInstance(ctx.getApplicationContext());
                appContext = ctx.getApplicationContext();
                Log.d(TAG, "CastContext initialized");
            }
        } catch (Throwable t) {
            Log.e(TAG, "Cast init failed", t);
        }
    }

    public static void onStart(Activity activity) {
        Log.d(TAG, "Cast onStart");
        if (activity == null) return;
        try {
            if (castContext == null) init(activity);
            if (sessionManager == null) {
                sessionManager = CastContext.getSharedInstance(activity).getSessionManager();
                Log.d(TAG, "Cast session manager activated");
            }
            if (!listenerRegistered && sessionManager != null) {
                sessionManager.addSessionManagerListener(sessionListener, CastSession.class);
                listenerRegistered = true;
                Log.d(TAG, "Cast listener registered");
            }
            CastSession s = (sessionManager != null) ? sessionManager.getCurrentCastSession() : null;
            if (s != null) { currentSession = s; attachStatusCallbackIfNeeded(); }
        } catch (Throwable t) {
            Log.e(TAG, "onStart failed", t);
        }
    }

    public static void onStop(Activity activity) {
        Log.d(TAG, "Cast onStop");
        // Keep listener attached across activity swaps; do not remove here.
    }

    public static void disconnect(Context ctx) {
        lastFinishedContentId = null;
        try {
            SessionManager sm = CastContext.getSharedInstance(ctx).getSessionManager();
            CastSession s = sm.getCurrentCastSession();
            if (s != null) sm.endCurrentSession(true);
        } catch (Throwable t) {
            Log.e(TAG, "disconnect failed", t);
        } finally {
            detachStatusCallback();
            cancelEndAlarm();
            stopKeepAliveService();
        }
    }

    // -------------------- Chooser --------------------

    public static void showChooser(Context ctx) {
        init(ctx);
        Activity activity = resolveActivity(ctx);
        if (activity == null) {
            Log.w(TAG, "showChooser(Context): no Activity available");
            return;
        }
        ensureListener(activity);

        CastSession s = (sessionManager != null) ? sessionManager.getCurrentCastSession() : null;
        if (s != null) {
            currentSession = s;
            attachStatusCallbackIfNeeded();
            if (s.isConnected()) {
                Log.d(TAG, "Already connected; not showing chooser.");
                return;
            }
        }

        activity.runOnUiThread(() -> {
            try {
                activity.startActivity(new Intent(activity, com.genesisx.cast.GXCastChooserActivity.class));
                Log.d(TAG, "Cast chooser activity launched");
            } catch (Throwable t) {
                Log.e(TAG, "showChooser failed", t);
            }
        });
    }

    // -------------------- Media load (legacy single item) --------------------
    public static void loadMedia(Context ctx, String url, String contentType, @Nullable String title, boolean autoplay) {
        Log.w(TAG, "loadMedia: Arrived");
        onMain(() -> {
            lastFinishedContentId = null;
            currentPlayingContentId = url;
            RemoteMediaClient c = client();
            if (c == null) { Log.w(TAG, "loadMedia: no RemoteMediaClient"); return; }

            attachStatusCallbackIfNeeded();

            int mediaType = MediaMetadata.MEDIA_TYPE_MOVIE; // or track if you prefer

            MediaMetadata meta = new MediaMetadata(mediaType);
            if (title != null) meta.putString(MediaMetadata.KEY_TITLE, title);

            MediaInfo media = new MediaInfo.Builder(url)
                    .setContentType(contentType)
                    .setStreamType(MediaInfo.STREAM_TYPE_BUFFERED)
                    .setMetadata(meta)
                    .build();

            Log.d(TAG, "loadMedia (legacy): " + contentType + " " + url);
            c.load(media, autoplay).setResultCallback(result -> {
                com.google.android.gms.common.api.Status st = result.getStatus();
                Log.d(TAG, "load() statusCode=" + st.getStatusCode());
                RemoteMediaClient rmc = client();
                if (rmc != null && rmc.getMediaStatus() != null) {
                    MediaStatus ms = rmc.getMediaStatus();
                    Log.d(TAG, "MediaStatus: " + ps(ms.getPlayerState())
                            + " idle=" + ir(ms.getIdleReason())
                            + " pos=" + ms.getStreamPosition()
                            + " url=" + (ms.getMediaInfo()!=null? ms.getMediaInfo().getContentId():"null"));
                }
            });
        });
    }

    // --------------- TODO: handle list -------------------
    // public static void loadMediaList(Context ctx, String json) throws org.json.JSONException {
    //     Log.w(TAG, "loadMedia: Arrived");
    //     onMain(() -> {
    //         RemoteMediaClient c = client();
    //         if (c == null) { Log.w(TAG, "loadMedia: no RemoteMediaClient"); return; }
    //         try {
    //             org.json.JSONArray a = new org.json.JSONArray(json);
    //             MediaQueueItem[] items = new MediaQueueItem[a.length()];

    //             attachStatusCallbackIfNeeded();

    //             for (int i = 0; i < a.length(); i++) {
    //                 org.json.JSONObject o = a.getJSONObject(i);
    //                 String url = o.getString("url");
    //                 String mime = o.getString("mime");
    //                 String title = o.getString("title");

    //                 MediaMetadata md = new MediaMetadata(MediaMetadata.MEDIA_TYPE_MUSIC_TRACK);
    //                 md.putString(MediaMetadata.KEY_TITLE, title);

    //                 MediaInfo mi = new MediaInfo.Builder(url)
    //                     .setContentType(mime)
    //                     .setStreamType(MediaInfo.STREAM_TYPE_BUFFERED)
    //                     .setMetadata(md)
    //                     .build();

    //                 items[i] = new MediaQueueItem.Builder(mi).setAutoplay(true).build();
    //             }

    //             c.queueLoad(items, 0, MediaStatus.REPEAT_MODE_REPEAT_ALL, null);
    //         } catch(org.json.JSONException e) {
    //             Log.e(TAG, "json array error");
    //             e.printStackTrace();
    //         }
    //     });
    // }

    // -------------------- Session listener --------------------
    private static final SessionManagerListener<CastSession> sessionListener =
            new SessionManagerListener<CastSession>() {
                @Override public void onSessionStarted(CastSession session, String sessionId) {
                    currentSession = session;
                    attachStatusCallbackIfNeeded();
                    notifyState(true, friendlyName(session));
                    startKeepAliveService();             // keep alive while connected
                    primeAlarmFromStatus();              // schedule first wake
                }
                @Override public void onSessionResumed(CastSession session, boolean wasSuspended) {
                    currentSession = session;
                    attachStatusCallbackIfNeeded();
                    notifyState(true, friendlyName(session));
                    startKeepAliveService();
                    primeAlarmFromStatus();
                }
                @Override public void onSessionEnding(CastSession session) {
                    detachStatusCallback();
                    notifyState(false, "session ending");
                }
                @Override public void onSessionEnded(CastSession session, int error) {
                    detachStatusCallback();
                    currentSession = null;
                    notifyState(false, "session ended");
                    cancelEndAlarm();
                    stopKeepAliveService();
                }
                @Override public void onSessionSuspended(CastSession session, int reason) {
                    notifyState(false, "suspended");
                }
                @Override public void onSessionStarting(CastSession session) { notifyState(true, friendlyName(session)); }
                @Override public void onSessionResuming(CastSession session, String s) { }
                @Override public void onSessionResumeFailed(CastSession session, int i) { }
                @Override public void onSessionStartFailed(CastSession session, int i) { }

                private String friendlyName(CastSession s) {
                    try { return s != null && s.getCastDevice() != null ? s.getCastDevice().getFriendlyName() : ""; }
                    catch (Exception e) { return ""; }
                }
            };

    private static void notifyState(final boolean connected, final String name) {
        Log.d(TAG, "notifyState connected=" + connected + " name=" + name);
        GXCastBridge.onConnectionChanged(connected, name);
    }

    private static void ensureListener(Activity activity) {
        if (activity == null) return;
        if (sessionManager == null) {
            sessionManager = CastContext.getSharedInstance(activity).getSessionManager();
            Log.d(TAG, "Cast session manager activated");
        }
        if (!listenerRegistered && sessionManager != null) {
            sessionManager.addSessionManagerListener(sessionListener, CastSession.class);
            listenerRegistered = true;
            Log.d(TAG, "Cast listener registered");
        }
    }

    // -------------------- Status callback (fixed, single) --------------------
    private static void attachStatusCallbackIfNeeded() {
        Log.d(TAG, "attachStatusCallbackIfNeeded");
        if (statusCbAttached) return;

        if (currentSession == null && sessionManager != null)
            currentSession = sessionManager.getCurrentCastSession();

        final RemoteMediaClient rmc = client();
        if (rmc == null) { Log.d(TAG, "no RemoteMediaClient"); return; }

        statusCb = new RemoteMediaClient.Callback() {
            @Override public void onStatusUpdated() {
                RemoteMediaClient c = client(); if (c == null) return;
                MediaStatus ms = c.getMediaStatus(); if (ms == null) return;

                // Now-playing detection
                int st = ms.getPlayerState();
                MediaInfo mi = ms.getMediaInfo();
                if ((st == MediaStatus.PLAYER_STATE_PLAYING || st == MediaStatus.PLAYER_STATE_BUFFERING)
                        && mi != null && mi.getContentId() != null) {
                    String cid = mi.getContentId();
                    if (!cid.equals(currentPlayingContentId)) {
                        currentPlayingContentId = cid;
                        lastFinishedContentId = null;
                        lastItemId = (ms.getCurrentItemId() != 0) ? ms.getCurrentItemId() : lastItemId;
                        Log.d(TAG, "Now playing: " + currentPlayingContentId + " itemId=" + lastItemId);
                    }
                }

                // Item change implies previous finished (unless cancel/error)
                int curItemId = ms.getCurrentItemId();
                if (lastItemId == null) lastItemId = (curItemId != 0) ? curItemId : null;
                if (curItemId != 0 && lastItemId != null && curItemId != lastItemId) {
                    int idle = ms.getIdleReason();
                    if (lastPlayerState != MediaStatus.PLAYER_STATE_IDLE
                            || (idle != MediaStatus.IDLE_REASON_CANCELED
                            && idle != MediaStatus.IDLE_REASON_ERROR)) {
                        String finishedId = (currentPlayingContentId != null) ? currentPlayingContentId : ("item:" + lastItemId);
                        currentPlayingContentId = null;
                        GXCastBridge.onTrackFinished(finishedId);
                        Log.d(TAG, "Finished by item-change: " + finishedId);
                    }
                    lastItemId = curItemId;
                }

                // Normal FINISHED
                if (st == MediaStatus.PLAYER_STATE_IDLE && ms.getIdleReason() == MediaStatus.IDLE_REASON_FINISHED) {
                    String finishedId = (currentPlayingContentId != null)
                            ? currentPlayingContentId
                            : (lastItemId != null ? ("item:" + lastItemId) : null);
                    if (finishedId != null && !finishedId.equals(lastFinishedContentId)) {
                        lastFinishedContentId = finishedId;
                        currentPlayingContentId = null;
                        GXCastBridge.onTrackFinished(finishedId);
                        Log.d(TAG, "Finished by FINISHED: " + finishedId);
                    }
                }

                // Schedule wake before expected end so we can persist progress even under doze
                primeAlarmFromStatus();

                lastPlayerState = st;
            }
        };

        rmc.registerCallback(statusCb);
        statusCbAttached = true;
        Log.d(TAG, "status callback attached");
    }

    private static void detachStatusCallback() {
        Log.d(TAG, "detachStatusCallback");
        if (!statusCbAttached) return;
        RemoteMediaClient rmc = client();
        if (rmc != null && statusCb != null) rmc.unregisterCallback(statusCb);
        statusCbAttached = false;
        statusCb = null;
        lastFinishedContentId = null;
    }

    // -------------------- Keep-alive service & alarm --------------------
    private static void startKeepAliveService() {
        if (appContext == null) return;
        Intent i = new Intent(appContext, GXCastKeepAliveService.class);
        if (Build.VERSION.SDK_INT >= 26) appContext.startForegroundService(i);
        else appContext.startService(i);
    }

    private static void stopKeepAliveService() {
        if (appContext == null) return;
        appContext.stopService(new Intent(appContext, GXCastKeepAliveService.class));
    }

    private static void primeAlarmFromStatus() {
        RemoteMediaClient c = client(); if (c == null) return;
        MediaStatus st = c.getMediaStatus(); if (st == null) return;
        long msLeft = estimatedMillisToEnd(st);
        String id = currentTrackId();
        scheduleEndAlarm(msLeft, id);
    }

    private static long estimatedMillisToEnd(MediaStatus st) {
        if (st == null) return 30_000;

        long dur = 0, pos = 0;
        try {
            if (st.getMediaInfo() != null) dur = st.getMediaInfo().getStreamDuration();
            // fallback to RemoteMediaClient position if available
            RemoteMediaClient c = client();
            if (c != null) pos = c.getApproximateStreamPosition();
        } catch (Exception ignore) {}

        long left = Math.max(0, dur - pos);
        int ps = st.getPlayerState();
        if (ps != MediaStatus.PLAYER_STATE_PLAYING) left = Math.min(left, 5_000);
        return left;
    }

    private static String currentTrackId() {
        try {
            RemoteMediaClient c = client();
            MediaInfo info = (c != null) ? c.getMediaInfo() : null;
            if (info == null) return "";
            MediaMetadata md = info.getMetadata();
            String id = (md != null && md.containsKey("trackId")) ? md.getString("trackId") : null;
            if (id != null) return id;
            return info.getContentId();
        } catch (Exception e) {
            return "";
        }
    }

    private static void scheduleEndAlarm(long millisUntilEnd, String trackId) {
        if (appContext == null) return;
        if (millisUntilEnd <= 1500) millisUntilEnd = 1500;
        long when = System.currentTimeMillis() + (millisUntilEnd - 1200);

        Intent intent = new Intent(appContext, com.genesisx.background.GXMediaCmdReceiver.class)
                .setAction(appContext.getPackageName() + ".GX_MEDIA_CMD")
                .putExtra("cmd", "cast_check_end")
                .putExtra("trackId", trackId);

        PendingIntent pi = PendingIntent.getBroadcast(
                appContext, REQ_CODE_END_PING, intent,
                PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);

        AlarmManager am = (AlarmManager) appContext.getSystemService(Context.ALARM_SERVICE);
        if (am == null) return;

        if (Build.VERSION.SDK_INT >= 23) {
            am.setExactAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, when, pi);
        } else {
            am.setExact(AlarmManager.RTC_WAKEUP, when, pi);
        }
    }

    private static void cancelEndAlarm() {
        if (appContext == null) return;

        Intent intent = new Intent(appContext, com.genesisx.background.GXMediaCmdReceiver.class)
                .setAction(appContext.getPackageName() + ".GX_MEDIA_CMD")
                .putExtra("cmd", "cast_check_end");

        PendingIntent pi = PendingIntent.getBroadcast(
                appContext, REQ_CODE_END_PING, intent,
                PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);

        AlarmManager am = (AlarmManager) appContext.getSystemService(Context.ALARM_SERVICE);
        if (am != null) am.cancel(pi);
    }

    // -------------------- Helpers --------------------
    private static void onMain(Runnable r) {
        if (Looper.myLooper() == Looper.getMainLooper()) r.run();
        else new Handler(Looper.getMainLooper()).post(r);
    }

    private static RemoteMediaClient client() {
        try {
            if (currentSession == null) return null;
            return currentSession.getRemoteMediaClient();
        } catch (Throwable ignored) { return null; }
    }

    private static Activity resolveActivity(Context ctx) {
        if (ctx instanceof Activity) return (Activity) ctx;
        try {
            Class<?> qtNative = Class.forName("org.qtproject.qt.android.QtNative");
            return (Activity) qtNative.getMethod("activity").invoke(null);
        } catch (Throwable ignored) { }
        return null;
    }

    private static String ps(int playerState) {
        switch (playerState) {
            case MediaStatus.PLAYER_STATE_IDLE: return "IDLE";
            case MediaStatus.PLAYER_STATE_PLAYING: return "PLAYING";
            case MediaStatus.PLAYER_STATE_PAUSED: return "PAUSED";
            case MediaStatus.PLAYER_STATE_BUFFERING: return "BUFFERING";
            default: return "UNKNOWN";
        }
    }
    private static String ir(int idleReason) {
        switch (idleReason) {
            case MediaStatus.IDLE_REASON_NONE: return "NONE";
            case MediaStatus.IDLE_REASON_FINISHED: return "FINISHED";
            case MediaStatus.IDLE_REASON_CANCELED: return "CANCELED";
            case MediaStatus.IDLE_REASON_INTERRUPTED: return "INTERRUPTED";
            case MediaStatus.IDLE_REASON_ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    // -------------------- Controls ----------------------
    public static void nextOnCast() {

    }
}


// package com.genesisx.cast;

// import android.os.Looper;
// import android.os.Handler;
// import android.os.Bundle;
// import android.app.Activity;
// import android.content.Context;
// import android.content.Intent;
// import android.util.Log;
// import android.widget.FrameLayout;
// import android.view.ContextThemeWrapper;

// import androidx.annotation.Nullable;
// import androidx.appcompat.app.AppCompatActivity;
// import androidx.mediarouter.app.MediaRouteButton;
// import androidx.mediarouter.media.MediaRouter;
// import androidx.mediarouter.media.MediaRouteSelector;

// import com.google.android.gms.cast.MediaInfo;
// import com.google.android.gms.cast.MediaMetadata;
// import com.google.android.gms.cast.framework.*;
// import com.google.android.gms.cast.framework.media.RemoteMediaClient;

// public final class GXCastManager {
//     private static final String TAG = "GXCastManager";
//     private static SessionManager sessionManager;
//     private static CastSession currentSession;

//     private static CastContext castContext;
//     private static MediaRouteSelector selector;
//     private static MediaRouteButton routeButton;
//     private static RemoteMediaClient.Callback statusCb;
//     private static boolean statusCbAttached = false;
//     private static String lastFinishedContentId = null;
//     private static String currentPlayingContentId = null;
//     private static Integer lastItemId = null;      // Cast queue item id we believe is “current”
//     private static Integer prevItemId = null;      // previous item id (to emit finished for)
//     private static int lastPlayerState = com.google.android.gms.cast.MediaStatus.PLAYER_STATE_UNKNOWN;

//     private static boolean listenerRegistered = false;

//     private static final int REQ_CODE_END_PING = 42;

//     private final Context ctx;
//     private RemoteMediaClient rmc;
//     private final RemoteMediaClient.Callback statusCb = new RemoteMediaClient.Callback() {
//         @Override public void onStatusUpdated() {
//             RemoteMediaClient c = rmc;
//             if (c == null) return;
//             MediaStatus st = c.getMediaStatus();
//             if (st == null) return;

//             // Save progress regularly (position can be read anytime)
//             saveProgressSafe();

//             int ps = st.getPlayerState();
//             int reason = st.getIdleReason();
//             if (ps == MediaStatus.PLAYER_STATE_IDLE &&
//                 reason == MediaStatus.IDLE_REASON_FINISHED) {
//                 // FINAL: last track finished on device
//                 onTrackFinished();
//             } else {
//                 // (Re)schedule end ping so we wake even if callback doesn’t fire later
//                 scheduleEndAlarm(estimatedMillisToEnd(st), currentTrackId());
//             }
//         }
//     };

//     public GXCastManager(Context appCtx) { this.ctx = appCtx.getApplicationContext(); }

//     public void onCastingStarted() {
//         CastSession s = CastContext.getSharedInstance(ctx).getSessionManager().getCurrentCastSession();
//         if (s == null) return;
//         rmc = s.getRemoteMediaClient();
//         if (rmc != null) rmc.registerCallback(statusCb);

//         // Keep the app process alive
//         startKeepAliveService();

//         // Prime a ping for the current item
//         MediaStatus st = (rmc != null) ? rmc.getMediaStatus() : null;
//         if (st != null) scheduleEndAlarm(estimatedMillisToEnd(st), currentTrackId());
//     }

//     public void onCastingEnded() {
//         if (rmc != null) rmc.unregisterCallback(statusCb);
//         rmc = null;
//         cancelEndAlarm();
//         stopKeepAliveService();
//     }

//     /* ====== Service control ====== */
//     private void startKeepAliveService() {
//         Intent i = new Intent(ctx, CastKeepAliveService.class);
//         if (Build.VERSION.SDK_INT >= 26) ctx.startForegroundService(i);
//         else ctx.startService(i);
//     }

//     private void stopKeepAliveService() {
//         ctx.stopService(new Intent(ctx, CastKeepAliveService.class));
//     }

//     /* ====== Alarm (re)planning ====== */
//     private void scheduleEndAlarm(long millisUntilEnd, String trackId) {
//         if (millisUntilEnd <= 1500) millisUntilEnd = 1500; // safety floor
//         long when = System.currentTimeMillis() + (millisUntilEnd - 1200); // ping ~1.2s before end

//         Intent intent = new Intent(ctx, com.genesisx.background.GXMediaCmdReceiver.class)
//                 .putExtra("cmd", "cast_check_end")
//                 .putExtra("trackId", trackId);

//         PendingIntent pi = PendingIntent.getBroadcast(
//                 ctx, REQ_CODE_END_PING, intent,
//                 PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);

//         AlarmManager am = (AlarmManager) ctx.getSystemService(Context.ALARM_SERVICE);
//         if (am == null) return;

//         if (Build.VERSION.SDK_INT >= 23) {
//             am.setExactAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, when, pi);
//         } else {
//             am.setExact(AlarmManager.RTC_WAKEUP, when, pi);
//         }
//     }

//     private void cancelEndAlarm() {
//         Intent intent = new Intent(ctx, com.genesisx.background.GXMediaCmdReceiver.class)
//                 .putExtra("cmd", "cast_check_end");
//         PendingIntent pi = PendingIntent.getBroadcast(
//                 ctx, REQ_CODE_END_PING, intent,
//                 PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);

//         AlarmManager am = (AlarmManager) ctx.getSystemService(Context.ALARM_SERVICE);
//         if (am != null) am.cancel(pi);
//     }

//     /* ====== Helpers ====== */
//     private long estimatedMillisToEnd(MediaStatus st) {
//         if (st == null) return 30_000;
//         long dur = st.getStreamDuration();        // ms
//         long pos = st.getStreamPosition();        // ms
//         long left = Math.max(0, dur - pos);
//         // If paused/buffering, pick a short re-check
//         int ps = st.getPlayerState();
//         if (ps != MediaStatus.PLAYER_STATE_PLAYING) left = Math.min(left, 5_000);
//         return left;
//     }

//     private String currentTrackId() {
//         try {
//             MediaInfo info = (rmc != null && rmc.getMediaInfo() != null) ? rmc.getMediaInfo() : null;
//             if (info == null) return "";
//             // Put your own identifier in customData when you enqueue:
//             // e.g. {"trackId":"abc123"}
//             MediaMetadata md = info.getMetadata();
//             String id = (md != null && md.containsKey("trackId")) ? md.getString("trackId") : null;
//             if (id != null) return id;
//             return info.getContentId(); // fallback
//         } catch (Exception e) {
//             return "";
//         }
//     }

//     private void saveProgressSafe() {
//         try {
//             if (rmc == null) return;
//             MediaStatus st = rmc.getMediaStatus();
//             if (st == null) return;
//             long pos = st.getStreamPosition();
//             String id = currentTrackId();
//             // Call into your Qt/JNI or background bridge:
//             // BackgroundBridge.dispatchCommand("cast_progress:" + id + ":" + pos);
//             // or nativeOnCastProgress(id, pos);
//         } catch (Exception ignore) {}
//     }

//     private void onTrackFinished() {
//         // Wake briefly to ensure DB write even if device is dozing
//         Intent i = new Intent(ctx, CastKeepAliveService.class);
//         // call pulse via startService and a receiver, or expose a static:
//         // simplest: just (re)start to ensure process is foregrounded
//         if (Build.VERSION.SDK_INT >= 26) ctx.startForegroundService(i); else ctx.startService(i);

//         // Save last played and advance your local model:
//         String id = currentTrackId();
//         // BackgroundBridge.dispatchCommand("cast_track_finished:" + id);
//         // nativeOnCastFinished(id);

//         // Cancel any pending alarm and schedule for next item (if still casting)
//         cancelEndAlarm();

//         // If queue is managed on Chromecast, nothing else to do.
//         // If phone orchestrates next, enqueue/signal here.
//     }

//     private GXCastManager() {}

//     public static void init(Context ctx) {
//         try {
//             if (castContext == null) {
//                 castContext = CastContext.getSharedInstance(ctx.getApplicationContext());
//                 selector = castContext.getMergedSelector();
//                 Log.d(TAG, "CastContext initialized");
//             }
//         } catch (Throwable t) {
//             Log.e(TAG, "Cast init failed", t);
//         }
//     }

//     public static void showChooser(Activity ctx) {
//         init(ctx);
//         Activity activity = resolveActivity(ctx);
//         if (activity == null) {
//             Log.w(TAG, "No Activity available to show Cast chooser");
//             return;
//         }
//         if (selector == null) {
//             Log.w(TAG, "Selector not ready");
//             return;
//         }

//         ensureListener(activity);

//         CastSession s = (sessionManager != null) ? sessionManager.getCurrentCastSession() : null;
//         if (s != null) {
//             currentSession = s;
//             attachStatusCallbackIfNeeded();
//             if (s.isConnected()) {
//                 Log.d(TAG, "Already connected; not showing chooser.");
//                 return;                      // prevents “route can’t be found” churn
//             }
//         }

//         activity.runOnUiThread(() -> {
//             try {
//                 activity.startActivity(
//                     new android.content.Intent(activity, com.genesisx.cast.GXCastChooserActivity.class));
//                 Log.d(TAG, "Cast chooser activity launched");
//             } catch (Throwable t) {
//                 Log.e(TAG, "showChooser failed", t);
//             }
//         });
//     }

//     public static void showChooser(android.content.Context ctx) {
//         init(ctx);
//         Activity activity = resolveActivity(ctx);
//         if (activity == null) {
//             Log.w(TAG, "showChooser(Context): no Activity available");
//             return;
//         }

//         ensureListener(activity);

//         CastSession s = (sessionManager != null) ? sessionManager.getCurrentCastSession() : null;
//         if (s != null) {
//             currentSession = s;
//             attachStatusCallbackIfNeeded();
//             if (s.isConnected()) {
//                 Log.d(TAG, "Already connected; not showing chooser.");
//                 return;                      // prevents “route can’t be found” churn
//             }
//         }
//         showChooser(activity); // delegate to the Activity version you already have
//     }

//     private static Activity resolveActivity(Context ctx) {
//         if (ctx instanceof Activity) return (Activity) ctx;
//         try {
//             Class<?> qtNative = Class.forName("org.qtproject.qt.android.QtNative");
//             Activity a = (Activity) qtNative.getMethod("activity").invoke(null);
//             return a;
//         } catch (Throwable ignored) {

//         }
//         return null;
//     }

//     private static final SessionManagerListener<CastSession> sessionListener =
//         new SessionManagerListener<CastSession>() {

//             @Override public void onSessionStarted(CastSession session, String sessionId) {
//                 currentSession = session;
//                 attachStatusCallbackIfNeeded();
//                 GXCastManager.notifyState(true, friendlyName(session));
//             }
//             @Override public void onSessionResumed(CastSession session, boolean wasSuspended) {
//                 currentSession = session;
//                 attachStatusCallbackIfNeeded();
//                 GXCastManager.notifyState(true, friendlyName(session));
//             }
//             @Override public void onSessionEnding(CastSession session) {
//                 detachStatusCallback();
//                 GXCastManager.notifyState(false, "session ending emmitted");
//             }
//             @Override public void onSessionEnded(CastSession session, int error) {
//                 detachStatusCallback();
//                 currentSession = null;
//                 GXCastManager.notifyState(false, "session ended emitted");
//             }
//             @Override public void onSessionSuspended(CastSession session, int reason) {
//                 GXCastManager.notifyState(false, "");
//             }
//             @Override public void onSessionStarting(CastSession session) { GXCastManager.notifyState(true, friendlyName(session)); }
//             @Override public void onSessionResuming(CastSession session, String s) { }
//             @Override public void onSessionResumeFailed(CastSession session, int i) { }
//             @Override public void onSessionStartFailed(CastSession session, int i) { }

//             private String friendlyName(CastSession s) {
//                 try {
//                     return s != null && s.getCastDevice() != null
//                         ? s.getCastDevice().getFriendlyName()
//                         : "";
//                 } catch (Exception e) {
//                     return "";
//                 }
//             }
//         };

//     private static void notifyState(final boolean connected, final String name) {
//         Log.d(TAG, "notifyState connected=" + connected + " name=" + name);
//         GXCastBridge.onConnectionChanged(connected, name);
//     }

//     private static void ensureListener(Activity activity) {
//         if (activity == null) return;
//         if (sessionManager == null) {
//             sessionManager = CastContext.getSharedInstance(activity).getSessionManager();
//             Log.d(TAG, "Cast session manager activated");
//         }
//         if (!listenerRegistered && sessionManager != null) {
//             sessionManager.addSessionManagerListener(sessionListener, CastSession.class);
//             listenerRegistered = true;
//             Log.d(TAG, "Cast listener registered");
//         }
//     }

//     private static void onMain(Runnable r) {
//         if (Looper.myLooper() == Looper.getMainLooper()) r.run();
//         else new Handler(Looper.getMainLooper()).post(r);
//     }

//     private static RemoteMediaClient client() {
//         try {
//             if (currentSession == null) return null;
//             return currentSession.getRemoteMediaClient();
//         } catch (Throwable ignored) { return null; }
//     }

//     public static void onStart(Activity activity) {
//         Log.d(TAG, "Cast onStart manager activated");
//         // sessionManager = CastContext.getSharedInstance(activity).getSessionManager();
//         // sessionManager.addSessionManagerListener(sessionListener, CastSession.class);
//         if (activity == null) return;
//         try {
//             if (sessionManager == null) {
//                 sessionManager = CastContext.getSharedInstance(activity).getSessionManager();
//                 Log.d(TAG, "Cast session manager activated");
//             }
//             if (!listenerRegistered && sessionManager != null) {
//                 sessionManager.addSessionManagerListener(sessionListener, CastSession.class);
//                 listenerRegistered = true;
//                 Log.d(TAG, "Cast listener registered");
//             }
//             CastSession s = (sessionManager != null) ? sessionManager.getCurrentCastSession() : null;
//             if (s != null) { currentSession = s; attachStatusCallbackIfNeeded(); }
//         } catch (Throwable t) {
//             Log.e(TAG, "onStart failed", t);
//         }
//     }

//     public static void onStop(Activity activity) {
//         Log.d(TAG, "Cast onStop manager activated");
//         if (sessionManager == null) return;
//         // try {
//         //     if (listenerRegistered) {
//         //         sessionManager.removeSessionManagerListener(sessionListener, CastSession.class);
//         //         listenerRegistered = false;
//         //         Log.d(TAG, "Cast listener unregistered");
//         //     }
//         // } catch (Throwable t) {
//         //     Log.e(TAG, "onStop failed", t);
//         // }
//     }

//     public static void disconnect(android.content.Context ctx) {
//         lastFinishedContentId = null;
//         try {
//             SessionManager sm = CastContext.getSharedInstance(ctx).getSessionManager();
//             CastSession s = sm.getCurrentCastSession();
//             if (s != null) sm.endCurrentSession(true);
//         } catch (Throwable t) {
//             Log.e("GXCastManager", "disconnect failed", t);
//         } finally {
//             detachStatusCallback();
//         }
//     }

//     public static void loadMedia(android.content.Context ctx, String url, String contentType, String title, boolean autoplay) {
//         Log.w(TAG, "loadMedia: Arrived");
//         onMain(() -> {
//             lastFinishedContentId = null;
//             currentPlayingContentId = url;
//             RemoteMediaClient c = client();
//             if (c == null) { Log.w(TAG, "loadMedia: no RemoteMediaClient"); return; }
//             Log.w(TAG, "loadMedia: RemoteMediaClient found");

//             attachStatusCallbackIfNeeded();

//             // int mediaType = contentType.startWith("audio/") ? MediaMetadata.MEDIA_TYPE_MUSIC_TRACK : MediaMetadata.MEDIA_TYPE_MOVIE;
//             int mediaType = MediaMetadata.MEDIA_TYPE_MOVIE;

//             MediaMetadata meta = new MediaMetadata(mediaType);
//             if (title != null) meta.putString(MediaMetadata.KEY_TITLE, title);
//             // "https://storage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4"
//             MediaInfo media = new MediaInfo.Builder(url)
//                 .setContentType(contentType)
//                 .setStreamType(MediaInfo.STREAM_TYPE_BUFFERED)
//                 .setMetadata(meta)
//                 .build();

//             Log.d(TAG, "loadMedia (legacy): " + contentType + " " + url);
//             c.load(media, autoplay).setResultCallback(result -> {
//                 com.google.android.gms.common.api.Status st = result.getStatus();
//                 Log.d(TAG, "load() result statusCode=" + st.getStatusCode());
//                 RemoteMediaClient rmc = client();
//                 if (rmc != null && rmc.getMediaStatus() != null) {
//                     com.google.android.gms.cast.MediaStatus ms = rmc.getMediaStatus();
//                     Log.d(TAG, "MediaStatus: " + ps(ms.getPlayerState())
//                             + " idle=" + ir(ms.getIdleReason())
//                             + " pos=" + ms.getStreamPosition()
//                             + " url=" + (ms.getMediaInfo()!=null? ms.getMediaInfo().getContentId():"null"));
//                 }
//             });

//             // boolean debugCallbacksAttached = false;

//             // if (!debugCallbacksAttached) {
//             //     RemoteMediaClient rmc = client(); if (rmc != null) {
//             //         rmc.registerCallback(new RemoteMediaClient.Callback() {
//             //             @Override public void onStatusUpdated() {
//             //                 com.google.android.gms.cast.MediaStatus ms = rmc.getMediaStatus();
//             //                 if (ms != null)
//             //                     Log.d(TAG, "onStatusUpdated: " + ps(ms.getPlayerState())
//             //                             + " idle=" + ir(ms.getIdleReason())
//             //                             + " pos=" + ms.getStreamPosition());
//             //             }
//             //         });
//             //         debugCallbacksAttached = true;
//             //     }
//             // }
//         });
//     }

//     private static String ps(int playerState) {
//         switch (playerState) {
//             case com.google.android.gms.cast.MediaStatus.PLAYER_STATE_IDLE: return "IDLE";
//             case com.google.android.gms.cast.MediaStatus.PLAYER_STATE_PLAYING: return "PLAYING";
//             case com.google.android.gms.cast.MediaStatus.PLAYER_STATE_PAUSED: return "PAUSED";
//             case com.google.android.gms.cast.MediaStatus.PLAYER_STATE_BUFFERING: return "BUFFERING";
//             case com.google.android.gms.cast.MediaStatus.PLAYER_STATE_UNKNOWN:
//             default: return "UNKNOWN";
//         }
//     }
//     private static String ir(int idleReason) {
//         switch (idleReason) {
//             case com.google.android.gms.cast.MediaStatus.IDLE_REASON_NONE: return "NONE";
//             case com.google.android.gms.cast.MediaStatus.IDLE_REASON_FINISHED: return "FINISHED";
//             case com.google.android.gms.cast.MediaStatus.IDLE_REASON_CANCELED: return "CANCELED";
//             case com.google.android.gms.cast.MediaStatus.IDLE_REASON_INTERRUPTED: return "INTERRUPTED";
//             case com.google.android.gms.cast.MediaStatus.IDLE_REASON_ERROR: return "ERROR";
//             default: return "UNKNOWN";
//         }
//     }

//     private static String currentContentId() {
//         Log.d(TAG, "currentContentId activated");
//         RemoteMediaClient c = client();
//         if (c == null || c.getMediaStatus() == null || c.getMediaStatus().getMediaInfo() == null) return null;
//         return c.getMediaStatus().getMediaInfo().getContentId();
//     }

//     private static void attachStatusCallbackIfNeeded() {
//         Log.d(TAG, "attachStatusCallbackIfNeeded");
//         if (statusCbAttached) return;                           // <-- restore guard

//         if (currentSession == null && sessionManager != null)   // fallback to pick up existing session
//             currentSession = sessionManager.getCurrentCastSession();

//         RemoteMediaClient rmc = client();
//         if (rmc == null) { Log.d(TAG, "no RemoteMediaClient"); return; }

//         statusCb = new RemoteMediaClient.Callback() {
//             @Override public void onStatusUpdated() {
//                 RemoteMediaClient c = client(); if (c == null) return;
//                 com.google.android.gms.cast.MediaStatus ms = c.getMediaStatus(); if (ms == null) return;

//                 int st = ms.getPlayerState();
//                 int idle = ms.getIdleReason();

//                 // 1) Detect item changes via queue item id (works even when there is no explicit FINISHED)
//                 int curItemId = ms.getCurrentItemId();   // <= key line

//                 if (lastItemId == null) {
//                     lastItemId = (curItemId != 0) ? curItemId : null;
//                 }

//                 if (curItemId != 0 && lastItemId != null && curItemId != lastItemId) {
//                     // Item changed → previous finished (or skipped). Treat as finished unless user canceled.
//                     if (lastPlayerState != com.google.android.gms.cast.MediaStatus.PLAYER_STATE_IDLE
//                         || (idle != com.google.android.gms.cast.MediaStatus.IDLE_REASON_CANCELED
//                             && idle != com.google.android.gms.cast.MediaStatus.IDLE_REASON_ERROR)) {

//                         prevItemId = lastItemId;
//                         lastItemId = curItemId;

//                         // Prefer a stable identifier to send to QML:
//                         String finishedId = (currentPlayingContentId != null) ? currentPlayingContentId : ("item:" + prevItemId);
//                         currentPlayingContentId = null;   // will be set again below when PLAYING/BFFERING arrives
//                         GXCastBridge.onTrackFinished(finishedId);
//                         Log.d(TAG, "Finished by item-change: " + finishedId);
//                     } else {
//                         // user canceled / error → just advance bookkeeping
//                         lastItemId = curItemId;
//                         currentPlayingContentId = null;
//                     }
//                 }

//                 // 2) Normal FINISHED path (when receiver reports it)
//                 if (st == com.google.android.gms.cast.MediaStatus.PLAYER_STATE_IDLE
//                     && idle == com.google.android.gms.cast.MediaStatus.IDLE_REASON_FINISHED) {

//                     String finishedId = (currentPlayingContentId != null) ? currentPlayingContentId
//                                       : (lastItemId != null ? ("item:" + lastItemId) : null);
//                     if (finishedId != null && !finishedId.equals(lastFinishedContentId)) {
//                         lastFinishedContentId = finishedId;
//                         currentPlayingContentId = null;
//                         GXCastBridge.onTrackFinished(finishedId);
//                         Log.d(TAG, "Finished by FINISHED: " + finishedId);
//                     }
//                 }

//                 // 3) Update “now playing” cache when PLAYING/BUFFERING
//                 com.google.android.gms.cast.MediaInfo mi = ms.getMediaInfo();
//                 if ((st == com.google.android.gms.cast.MediaStatus.PLAYER_STATE_PLAYING
//                   || st == com.google.android.gms.cast.MediaStatus.PLAYER_STATE_BUFFERING)
//                   && mi != null && mi.getContentId() != null) {

//                     String cid = mi.getContentId();
//                     if (!cid.equals(currentPlayingContentId)) {
//                         currentPlayingContentId = cid;
//                         lastFinishedContentId = null;
//                         lastItemId = (ms.getCurrentItemId() != 0) ? ms.getCurrentItemId() : lastItemId;
//                         Log.d(TAG, "Now playing: " + currentPlayingContentId + " itemId=" + lastItemId);
//                     }
//                 }

//                 // 4) Remember last player state
//                 lastPlayerState = st;
//                 // RemoteMediaClient c = client(); if (c == null) return;
//                 // com.google.android.gms.cast.MediaStatus ms = c.getMediaStatus(); if (ms == null) return;

//                 // // 1) Track current playing item as soon as we see PLAYING/BUFFERING
//                 // int st = ms.getPlayerState();
//                 // com.google.android.gms.cast.MediaInfo mi = ms.getMediaInfo();

//                 // if ((st == com.google.android.gms.cast.MediaStatus.PLAYER_STATE_PLAYING
//                 //   || st == com.google.android.gms.cast.MediaStatus.PLAYER_STATE_BUFFERING)
//                 //   && mi != null && mi.getContentId() != null) {

//                 //     String cid = mi.getContentId();
//                 //     if (!cid.equals(currentPlayingContentId)) {
//                 //         currentPlayingContentId = cid;
//                 //         lastFinishedContentId   = null;    // reset debounce for the new item
//                 //         Log.d(TAG, "Now playing: " + currentPlayingContentId);
//                 //     }
//                 // }

//                 // // 2) Detect end-of-item (IDLE + FINISHED)
//                 // if ((st == com.google.android.gms.cast.MediaStatus.PLAYER_STATE_IDLE || st == com.google.android.gms.cast.MediaStatus.PLAYER_STATE_UNKNOWN)
//                 //         && ms.getIdleReason() == com.google.android.gms.cast.MediaStatus.IDLE_REASON_FINISHED) {

//                 //     String finishedId = currentPlayingContentId;   // use cached id
//                 //     Log.d(TAG, "IDLE/FINISHED for " + finishedId);

//                 //     if (finishedId != null && !finishedId.equals(lastFinishedContentId)) {  // <-- debounce
//                 //         lastFinishedContentId   = finishedId;
//                 //         currentPlayingContentId = null;           // ready for next item
//                 //         GXCastBridge.onTrackFinished(finishedId); // JNI → QML
//                 //     }
//                 // }

//                 // // 3) On cancel/error: clear current so next PLAYING sets it
//                 // if (st == com.google.android.gms.cast.MediaStatus.PLAYER_STATE_IDLE) {
//                 //     int reason = ms.getIdleReason();
//                 //     if (reason == com.google.android.gms.cast.MediaStatus.IDLE_REASON_CANCELED
//                 //      || reason == com.google.android.gms.cast.MediaStatus.IDLE_REASON_ERROR
//                 //      || reason == com.google.android.gms.cast.MediaStatus.IDLE_REASON_INTERRUPTED) {
//                 //         currentPlayingContentId = null;
//                 //     }
//                 // }
//             }
//         };

//         rmc.registerCallback(statusCb);
//         statusCbAttached = true;
//         Log.d(TAG, "status callback attached");
//     }


//     // private static void attachStatusCallbackIfNeeded() {
//     //     Log.d(TAG, "attachStatusCallbackIfNeeded activated");
//     //     // if (statusCbAttached) return;
//     //     RemoteMediaClient rmc = client();
//     //     if (rmc == null) return;
//     //     Log.d(TAG, "attachStatusCallbackIfNeeded second");
//     //     statusCb = new RemoteMediaClient.Callback() {
//     //         @Override public void onStatusUpdated() {
//     //             Log.d(TAG, "attachStatusCallbackIfNeeded third");
//     //             RemoteMediaClient c = client();
//     //             if (c == null) return;
//     //             com.google.android.gms.cast.MediaStatus ms = c.getMediaStatus();
//     //             if (ms == null) return;
//     //             Log.d(TAG, "attachStatusCallbackIfNeeded fourth");
//     //             // Track "now playing" as soon as we see it (PLAYING/BUFFERING with non-null MediaInfo)
//     //             if ((ms.getPlayerState() == com.google.android.gms.cast.MediaStatus.PLAYER_STATE_PLAYING
//     //                 || ms.getPlayerState() == com.google.android.gms.cast.MediaStatus.PLAYER_STATE_BUFFERING)
//     //                 && ms.getMediaInfo() != null && ms.getMediaInfo().getContentId() != null) {
//     //                 Log.d(TAG, "attachStatusCallbackIfNeeded fifth");
//     //                 String cid = ms.getMediaInfo().getContentId();
//     //                 if (!cid.equals(currentPlayingContentId)) {
//     //                     // NEW item started: clear the debounce for the new one
//     //                     currentPlayingContentId = cid;       // NEW
//     //                     lastFinishedContentId   = null;      // NEW
//     //                     Log.d(TAG, "Now playing: " + currentPlayingContentId);
//     //                 } else {
//     //                     Log.d(TAG, "attachStatusCallbackIfNeeded seventh=" + currentPlayingContentId + "and cid=" + cid);
//     //                 }
//     //             } else {
//     //                 Log.d(TAG, "attachStatusCallbackIfNeeded sixth info=" + ms.getMediaInfo() + "and contentId=" + ms.getMediaInfo().getContentId());
//     //             }
//     //             Log.d(TAG, "attachStatusCallbackIfNeeded final check, how many times??????");
//     //             // Detect end-of-item reliably
//     //             if (ms.getPlayerState() == com.google.android.gms.cast.MediaStatus.PLAYER_STATE_IDLE
//     //                     && ms.getIdleReason() == com.google.android.gms.cast.MediaStatus.IDLE_REASON_FINISHED) {

//     //                 // Use cached id; MediaInfo may be null when idle
//     //                 String finishedId = currentPlayingContentId;           // NEW
//     //                 Log.d(TAG, "IDLE/FINISHED for " + finishedId);

//     //                 // if (finishedId != null && !finishedId.equals(lastFinishedContentId)) {
//     //                     lastFinishedContentId = finishedId;                // debounce per item
//     //                     // Clear current playing so a subsequent PLAYING sets it for the next track
//     //                     currentPlayingContentId = null;                    // NEW
//     //                     GXCastBridge.onTrackFinished(finishedId);          // JNI → QML
//     //                 // }
//     //             } else {
//     //                 Log.d(TAG, "attachStatusCallbackIfNeeded eighth");
//     //             }

//     //             // (optional) still forward player status if you use it:
//     //             // GXCastBridge.onPlayerStatus(ms.getPlayerState(), ms.getStreamPosition(), ms.getStreamDuration());
//     //         }
//     //         // @Override public void onStatusUpdated() {
//     //         //     RemoteMediaClient c = client();
//     //         //     if (c == null) return;
//     //         //     com.google.android.gms.cast.MediaStatus ms = c.getMediaStatus();
//     //         //     if (ms == null) return;

//     //         //     // existing status -> QML (optional)
//     //         //     long pos = ms.getStreamPosition();
//     //         //     int  st  = ms.getPlayerState();    // PLAYING=1, PAUSED=2, BUFFERING=3, IDLE=4
//     //         //     // GXCastBridge.onPlayerStatus(st, pos);

//     //         //     // NEW: detect end-of-item
//     //         //     if (st == com.google.android.gms.cast.MediaStatus.PLAYER_STATE_IDLE
//     //         //             && ms.getIdleReason() == com.google.android.gms.cast.MediaStatus.IDLE_REASON_FINISHED) {

//     //         //         String cid = currentContentId();
//     //         //         Log.d(TAG, "got end of item=" + cid);
//     //         //         if (cid != null && !cid.equals(lastFinishedContentId)) {
//     //         //             lastFinishedContentId = cid;           // debounce per item
//     //         //             GXCastBridge.onTrackFinished(cid);     // -> JNI -> QML
//     //         //         }
//     //         //     }
//     //         // }
//     //     };
//     //     rmc.registerCallback(statusCb);
//     //     statusCbAttached = true;
//     // }

//     private static void detachStatusCallback() {
//         Log.d(TAG, "detach StatusCallback activated");
//         if (!statusCbAttached) return;
//         RemoteMediaClient rmc = client();
//         if (rmc != null && statusCb != null) rmc.unregisterCallback(statusCb);
//         statusCbAttached = false;
//         statusCb = null;
//         lastFinishedContentId = null;
//     }
// }
