// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

package permissions;

import android.app.Activity;
import android.app.AlarmManager;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.provider.Settings;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.app.NotificationManagerCompat;
import androidx.core.content.ContextCompat;

public final class GxPermissions {
    private GxPermissions() {}

    // ===== Generic checks + requests =====
    public static boolean has(@NonNull Activity a, @NonNull String permission) {
        return ContextCompat.checkSelfPermission(a, permission) == PackageManager.PERMISSION_GRANTED;
    }

    public static void request(@NonNull Activity a, @NonNull String[] permissions) {
        ActivityCompat.requestPermissions(a, permissions, 7001);
    }

    // ===== Notifications =====
    public static boolean areNotificationsEnabled(@NonNull Activity a) {
        return NotificationManagerCompat.from(a).areNotificationsEnabled();
    }

    public static void requestNotifications(@NonNull Activity a) {
        if (Build.VERSION.SDK_INT >= 33) {
            if (ContextCompat.checkSelfPermission(a, "android.permission.POST_NOTIFICATIONS")
                    != android.content.pm.PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(a, new String[]{"android.permission.POST_NOTIFICATIONS"}, 7002);
            }
        } else {
            if (!areNotificationsEnabled(a)) {
                openNotificationSettings(a);
            }
        }
    }

    public static void openNotificationSettings(@NonNull Activity a) {
        Intent i = new Intent();
        if (Build.VERSION.SDK_INT >= 26) {
            i.setAction(Settings.ACTION_APP_NOTIFICATION_SETTINGS);
            i.putExtra(Settings.EXTRA_APP_PACKAGE, a.getPackageName());
        } else {
            i.setAction(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
            i.setData(Uri.fromParts("package", a.getPackageName(), null));
        }
        a.startActivity(i);
    }

    // ===== App settings =====
    public static void openAppSettings(@NonNull Activity a) {
        Intent i = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
        i.setData(Uri.fromParts("package", a.getPackageName(), null));
        a.startActivity(i);
    }

    // ===== Special flows (no runtime dialog) =====
    public static boolean canScheduleExactAlarms(@NonNull Activity a) {
        if (Build.VERSION.SDK_INT < 31) return true;
        AlarmManager am = (AlarmManager) a.getSystemService(Activity.ALARM_SERVICE);
        return am != null && am.canScheduleExactAlarms();
    }

    public static void requestScheduleExactAlarms(@NonNull Activity a) {
        if (Build.VERSION.SDK_INT >= 31) {
            Intent i = new Intent(Settings.ACTION_REQUEST_SCHEDULE_EXACT_ALARM);
            i.setData(Uri.fromParts("package", a.getPackageName(), null));
            a.startActivity(i);
        }
    }

    // Battery optimizations (Doze)
    public static boolean isIgnoringBatteryOptimizations(@NonNull Activity a) {
        if (Build.VERSION.SDK_INT < 23) return true;
        return a.getSystemService(android.os.PowerManager.class)
                .isIgnoringBatteryOptimizations(a.getPackageName());
    }

    public static void requestIgnoreBatteryOptimizations(@NonNull Activity a) {
        if (Build.VERSION.SDK_INT >= 23) {
            Intent i = new Intent(Settings.ACTION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS);
            i.setData(Uri.parse("package:" + a.getPackageName()));
            a.startActivity(i);
        }
    }

    // ===== Settings-based "permissions" =====
    public static void openOverlaySettings(Activity a) {
        if (a == null) return;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            Intent i = new Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION,
                    Uri.parse("package:" + a.getPackageName()));
            a.startActivity(i);
        }
    }

    // All files access
    public static boolean hasAllFilesAccess() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R) return true;
        try {
            return android.os.Environment.isExternalStorageManager();
        } catch (Throwable t) {
            return false;
        }
    }

    public static void openAllFilesAccessSettings(Activity a) {
        if (a == null) return;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            Intent i = new Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION);
            i.setData(Uri.parse("package:" + a.getPackageName()));
            a.startActivity(i);
        }
    }

    // Unknown sources
    public static void openUnknownSourcesSettings(Activity a) {
        if (a == null) return;
        Intent i = new Intent(Settings.ACTION_MANAGE_UNKNOWN_APP_SOURCES,
                Uri.parse("package:" + a.getPackageName()));
        a.startActivity(i);
    }

    // Do Not Disturb policy
    public static void openDndPolicySettings(Activity a) {
        if (a == null) return;
        Intent i = new Intent(Settings.ACTION_NOTIFICATION_POLICY_ACCESS_SETTINGS);
        a.startActivity(i);
    }
}
