// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#include <QList>
#include <QObject>
#include <QString>
#include <QtGlobal>
#include <QVariant>

#include <GenesisX/genesisx_global.h>

using namespace Qt::StringLiterals;

#ifdef Q_OS_ANDROID
bool gx_app_has_android(const QString& permission);
void gx_app_request_android(const QStringList& list);
bool gx_app_permissions_notifications_enabled_android();
void gx_app_permissions_request_notifications_android();
void gx_app_permissions_open_notifications_settings_android();
void gx_app_open_settings_android();
bool gx_app_can_exact_alarm_android();
void gx_app_request_exact_alarm_android();
bool gx_app_is_ignoring_batt_opt_android();
void gx_app_request_ignore_batt_opt_android();

void gx_app_open_overlay_settings_android();
bool gx_app_has_all_files_access_android();
void gx_app_open_all_files_access_settings_android();
void gx_app_open_unknown_sources_settings_android();
void gx_app_open_dnd_policy_settings_android();
#endif

namespace gx::app::permissions {

class GENESISX_CORE_EXPORT Permissions : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool notificationsEnabled READ notificationsEnabled NOTIFY notificationsEnabledChanged)

    Q_PROPERTY(QString CAMERA READ camera CONSTANT)
    Q_PROPERTY(QString RECORD_AUDIO READ recordAudio CONSTANT)
    Q_PROPERTY(QString ACCESS_FINE_LOCATION READ fineLocation CONSTANT)
    Q_PROPERTY(QString ACCESS_COARSE_LOCATION READ coarseLocation CONSTANT)
    Q_PROPERTY(QString BLUETOOTH_CONNECT READ btConnect CONSTANT)
    Q_PROPERTY(QString BLUETOOTH_SCAN READ btScan CONSTANT)
    Q_PROPERTY(QString READ_MEDIA_IMAGES READ readMediaImages CONSTANT)
    Q_PROPERTY(QString READ_MEDIA_VIDEO READ readMediaVideo CONSTANT)
    Q_PROPERTY(QString READ_MEDIA_AUDIO READ readMediaAudio CONSTANT)

    Q_PROPERTY(QString READ_MEDIA_VISUAL_USER_SELECTED READ readMediaUserSelected CONSTANT)
    Q_PROPERTY(QString ACCESS_BACKGROUND_LOCATION READ accessBackgroundLocation CONSTANT)
    Q_PROPERTY(QString ACTIVITY_RECOGNITION READ activityRecognition CONSTANT)
    Q_PROPERTY(QString BODY_SENSORS READ bodySensors CONSTANT)
    Q_PROPERTY(QString BODY_SENSORS_BACKGROUND READ bodySensorsBackground CONSTANT)
    Q_PROPERTY(QString NEARBY_WIFI_DEVICES READ nearbyWifiDevices CONSTANT)

public:
    explicit Permissions(QObject* parent = nullptr);

    Q_INVOKABLE bool has(const QString& permission) const {
#ifdef Q_OS_ANDROID
        return gx_app_has_android(permission);
#else
        Q_UNUSED(permission);
        return true;
#endif
    }

    Q_INVOKABLE void request(const QVariant& perms) {
        Q_UNUSED(perms);
#ifdef Q_OS_ANDROID
        QStringList list;
        if (perms.canConvert<QString>())
            list << perms.toString();
        else if (perms.canConvert<QStringList>())
            list = perms.toStringList();
        else if (perms.typeId() == QMetaType::QVariantList) {
            for (const auto& v : perms.toList()) list << v.toString();
        }
        if (!list.isEmpty()) gx_app_request_android(list);
#endif
    }

    bool notificationsEnabled() const {
#ifdef Q_OS_ANDROID
        return gx_app_permissions_notifications_enabled_android();
#else
        return true;
#endif
    }

    Q_INVOKABLE void requestNotifications() {
#ifdef Q_OS_ANDROID
        gx_app_permissions_request_notifications_android();
#endif
    }

    Q_INVOKABLE void openNotificationSettings() {
#ifdef Q_OS_ANDROID
        gx_app_permissions_open_notifications_settings_android();
#endif
    }

    Q_INVOKABLE void openAppSettings() {
#ifdef Q_OS_ANDROID
        gx_app_open_settings_android();
#endif
    }

    Q_INVOKABLE bool canScheduleExactAlarms() const {
#ifdef Q_OS_ANDROID
        return gx_app_can_exact_alarm_android();
#else
        return true;
#endif
    }

    Q_INVOKABLE void requestScheduleExactAlarms() {
#ifdef Q_OS_ANDROID
        gx_app_request_exact_alarm_android();
#endif
    }

    Q_INVOKABLE bool isIgnoringBatteryOptimizations() const {
#ifdef Q_OS_ANDROID
        return gx_app_is_ignoring_batt_opt_android();
#else
        return true;
#endif
    }

    Q_INVOKABLE void requestIgnoreBatteryOptimizations() {
#ifdef Q_OS_ANDROID
        gx_app_request_ignore_batt_opt_android();
#endif
    }

    QString camera() const { return u"android.permission.CAMERA"_s; }
    QString recordAudio() const { return u"android.permission.RECORD_AUDIO"_s; }
    QString fineLocation() const { return u"android.permission.ACCESS_FINE_LOCATION"_s; }
    QString coarseLocation() const { return u"android.permission.ACCESS_COARSE_LOCATION"_s; }
    QString btConnect() const { return u"android.permission.BLUETOOTH_CONNECT"_s; }     // API 31+
    QString btScan() const { return u"android.permission.BLUETOOTH_SCAN"_s; }           // API 31+
    QString readMediaImages() const { return u"android.permission.READ_MEDIA_IMAGES"_s; } // API 33+
    QString readMediaVideo() const { return u"android.permission.READ_MEDIA_VIDEO"_s; }   // API 33+
    QString readMediaAudio() const { return u"android.permission.READ_MEDIA_AUDIO"_s; }   // API 33+

    QString readMediaUserSelected() const { return u"android.permission.READ_MEDIA_VISUAL_USER_SELECTED"_s; } // API 34+
    QString accessBackgroundLocation() const { return u"android.permission.ACCESS_BACKGROUND_LOCATION"_s; }
    QString activityRecognition() const { return u"android.permission.ACTIVITY_RECOGNITION"_s; }
    QString bodySensors() const { return u"android.permission.BODY_SENSORS"_s; }
    QString bodySensorsBackground() const { return u"android.permission.BODY_SENSORS_BACKGROUND"_s; } // API 31+
    QString nearbyWifiDevices() const { return u"android.permission.NEARBY_WIFI_DEVICES"_s; }


signals:
    void notificationsEnabledChanged();
};

}

#endif // PERMISSIONS_H
