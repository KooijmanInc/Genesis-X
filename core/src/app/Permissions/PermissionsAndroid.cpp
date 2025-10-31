// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <QtGlobal>

#ifdef Q_OS_ANDROID
#include <QtCore/QJniObject>
#include <QtCore/QCoreApplication>

static QJniObject activity()
{
    return QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "activity", "()Landroid/app/Activity;");
}

bool gx_app_has_android(const QString &permission)
{
    QJniObject act = activity();
    if (!act.isValid()) return true;
    QJniObject jperm = QJniObject::fromString(permission);
    return QJniObject::callStaticMethod<jboolean>(
        "permissions/GxPermissions", "has",
        "(Landroid/app/Activity;Ljava/lang/String;)Z",
        act.object<jobject>(), jperm.object<jstring>());
}

void gx_app_request_android(const QStringList &permissions)
{
    QJniObject act = QJniObject::callStaticObjectMethod(
        "org/qtproject/qt/android/QtNative", "activity", "()Landroid/app/Activity;");
    if (!act.isValid() || permissions.isEmpty())
        return;

    QJniEnvironment env;                          // wrapper you want
    JNIEnv *e = env.jniEnv();                     // get JNIEnv*

    // Build Java String[] from QStringList
    jclass stringCls = e->FindClass("java/lang/String");
    jobjectArray arr = e->NewObjectArray(static_cast<jsize>(permissions.size()),
                                         stringCls, nullptr);

    for (jsize i = 0; i < static_cast<jsize>(permissions.size()); ++i) {
        QJniObject jstr = QJniObject::fromString(permissions.at(i));
        e->SetObjectArrayElement(arr, i, jstr.object<jstring>());
    }

    // Call GxPermissions.request(Activity, String[])
    QJniObject::callStaticMethod<void>(
        "permissions/GxPermissions", "request",
        "(Landroid/app/Activity;[Ljava/lang/String;)V",
        act.object<jobject>(), arr);

    // Clean up local refs
    e->DeleteLocalRef(arr);
    e->DeleteLocalRef(stringCls);
}

// static QJniObject currentActivity()
// {
//     // calls org.qtproject.qt.android.QtNative.activity(): Activity
//     return QJniObject::callStaticObjectMethod(
//         "org/qtproject/qt/android/QtNative",
//         "activity",
//         "()Landroid/app/Activity;");
// }

bool gx_app_permissions_notifications_enabled_android()
{
    QJniObject act = activity();
    if (!act.isValid())
        return true; // be lenient if we can’t get an Activity

    jboolean enabled = QJniObject::callStaticMethod<jboolean>(
        "permissions/GxPermissions",
        "areNotificationsEnabled",
        "(Landroid/app/Activity;)Z",
        act.object<jobject>());     // <-- pass Activity jobject

    return enabled;
}

void gx_app_permissions_request_notifications_android()
{
    QJniObject act = activity();
    if (!act.isValid())
        return;

    QJniObject::callStaticMethod<void>(
        "permissions/GxPermissions",
        "requestNotifications",
        "(Landroid/app/Activity;)V",
        act.object<jobject>());     // <-- pass Activity jobject
}

void gx_app_permissions_open_notifications_settings_android()
{
    QJniObject act = activity();
    if (!act.isValid())
        return;

    QJniObject::callStaticMethod<void>(
        "permissions/GxPermissions",
        "openNotificationSettings",
        "(Landroid/app/Activity;)V",
        act.object<jobject>());     // <-- pass Activity jobject
}

void gx_app_open_settings_android()
{
    QJniObject act = activity();
    if (!act.isValid()) return;
    QJniObject::callStaticMethod<void>(
        "permissions/GxPermissions", "openAppSettings",
        "(Landroid/app/Activity;)V", act.object<jobject>());
}

bool gx_app_can_exact_alarm_android()
{
    QJniObject act = activity();
    if (!act.isValid()) return true;
    return QJniObject::callStaticMethod<jboolean>(
        "permissions/GxPermissions", "canScheduleExactAlarms",
        "(Landroid/app/Activity;)Z", act.object<jobject>());
}

void gx_app_request_exact_alarm_android()
{
    QJniObject act = activity();
    if (!act.isValid()) return;
    QJniObject::callStaticMethod<void>(
        "permissions/GxPermissions", "requestScheduleExactAlarms",
        "(Landroid/app/Activity;)V", act.object<jobject>());
}

bool gx_app_is_ignoring_batt_opt_android()
{
    QJniObject act = activity();
    if (!act.isValid()) return true;
    return QJniObject::callStaticMethod<jboolean>(
        "permissions/GxPermissions", "isIgnoringBatteryOptimizations",
        "(Landroid/app/Activity;)Z", act.object<jobject>());
}

void gx_app_request_ignore_batt_opt_android()
{
    QJniObject act = activity();
    if (!act.isValid()) return;
    QJniObject::callStaticMethod<void>(
        "permissions/GxPermissions", "requestIgnoreBatteryOptimizations",
        "(Landroid/app/Activity;)V", act.object<jobject>());
}

// ---------- Settings helpers ----------
void gx_app_open_overlay_settings_android()
{
    QJniObject act = QJniObject::callStaticObjectMethod(
        "org/qtproject/qt/android/QtNative", "activity", "()Landroid/app/Activity;");
    if (!act.isValid()) return;

    QJniObject::callStaticMethod<void>(
        "permissions/GxPermissions", "openOverlaySettings",
        "(Landroid/app/Activity;)V",
        act.object<jobject>());
}

bool gx_app_has_all_files_access_android()
{
#ifdef Q_OS_ANDROID
    return QJniObject::callStaticMethod<jboolean>(
        "permissions/GxPermissions", "hasAllFilesAccess", "()Z");
#else
    return false;
#endif
}

void gx_app_open_all_files_access_settings_android()
{
    QJniObject act = QJniObject::callStaticObjectMethod(
        "org/qtproject/qt/android/QtNative", "activity", "()Landroid/app/Activity;");
    if (!act.isValid()) return;

    QJniObject::callStaticMethod<void>(
        "permissions/GxPermissions", "openAllFilesAccessSettings",
        "(Landroid/app/Activity;)V",
        act.object<jobject>());
}

void gx_app_open_unknown_sources_settings_android()
{
    QJniObject act = QJniObject::callStaticObjectMethod(
        "org/qtproject/qt/android/QtNative", "activity", "()Landroid/app/Activity;");
    if (!act.isValid()) return;

    QJniObject::callStaticMethod<void>(
        "permissions/GxPermissions", "openUnknownSourcesSettings",
        "(Landroid/app/Activity;)V",
        act.object<jobject>());
}

void gx_app_open_dnd_policy_settings_android()
{
    QJniObject act = QJniObject::callStaticObjectMethod(
        "org/qtproject/qt/android/QtNative", "activity", "()Landroid/app/Activity;");
    if (!act.isValid()) return;

    QJniObject::callStaticMethod<void>(
        "permissions/GxPermissions", "openDndPolicySettings",
        "(Landroid/app/Activity;)V",
        act.object<jobject>());
}

#endif
