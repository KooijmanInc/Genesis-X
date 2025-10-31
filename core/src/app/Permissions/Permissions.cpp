// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Permissions/Permissions.h>

/*!
    \class gx::app::permissions::Permissions
    \inheaderfile ../../../include/GenesisX/Permissions/Permissions.h
    \inmodule GenesisX
    \ingroup genesisx-core
    \title Mobile Permissions
    \since 6.10
    \brief Setting permissions for mobile devices.
 */

/*!
    \qmlmodule GenesisX.App.Permissions 1.0
    \title Genesis-X Permissions (QML)
    \brief QML APIs for permissions.

    Import this module to use the \l Permissions type:
    \code
    import GenesisX.App.Permissions 1.0
    \endcode
 */

/*!
    \qmltype Permissions
    \inqmlmodule GenesisX.App.Permissions
    \since GenesisX.App.Permissions 1.0
    \brief QML APIs for permissions.

    \section2 Example
    \qml
    import GenesisX.App.Permissions

    Permissions {
        Component.onCompleted: {
            if (!perms.notificationsEnabled) {
                perms.requestNotifications()
            }

            const need = []
            if (!perms.has(perms.CAMERA)) need.push(perms.CAMERA)
            if (!perms.has(perms.READ_MEDIA_IMAGES)) need.push(perms.READ_MEDIA_IMAGES)
            if (need.length) perms.request(need)
        }
    }
    \endqml

    \section2 Properties
    \qmlproperty string Permissions::notificationsEnabled
    \qmlproperty string Permissions::CAMERA
    \qmlproperty string Permissions::RECORD_AUDIO
    \qmlproperty string Permissions::ACCESS_FINE_LOCATION
    \qmlproperty string Permissions::ACCESS_COARSE_LOCATION
    \qmlproperty string Permissions::BLUETOOTH_CONNECT
    \qmlproperty string Permissions::BLUETOOTH_SCAN
    \qmlproperty string Permissions::READ_MEDIA_IMAGES
    \qmlproperty string Permissions::READ_MEDIA_VIDEO
    \qmlproperty string Permissions::READ_MEDIA_AUDIO
    \qmlproperty string Permissions::READ_MEDIA_VISUAL_USER_SELECTED
    \qmlproperty string Permissions::ACCESS_BACKGROUND_LOCATION
    \qmlproperty string Permissions::ACTIVITY_RECOGNITION
    \qmlproperty string Permissions::BODY_SENSORS
    \qmlproperty string Permissions::BODY_SENSORS_BACKGROUND
    \qmlproperty string Permissions::NEARBY_WIFI_DEVICES

    \section2 Signals
    \qmlsignal void Permissions::notificationEnabledChanged()

    \section2 Methods
    \qmlmethod bool Permissions::has(string permission)
    \qmlmethod void Permissions::request(var perms)
    \qmlmethod void Permissions::requestNotifications()
    \qmlmethod void Permissions::openNotificationSettings()
    \qmlmethod void Permissions::openAppSettings()
    \qmlmethod bool Permissions::canScheduleExactAlarms()
    \qmlmethod void Permissions::requestScheduleExactAlarms()
    \qmlmethod bool Permissions::isIgnoringBatteryOptimizations()
    \qmlmethod void Permissions::requestignoreBatteryOptimizations()
 */

using namespace gx::app::permissions;

Permissions::Permissions(QObject *parent)
    : QObject{parent}
{
}
