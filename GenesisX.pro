# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

TEMPLATE = subdirs
CONFIG += qt c++23
CONFIG += ordered

SUBDIRS += core physics orm \

!android:!ios {
    SUBDIRS += tools/gxgen
}

core.subdir = $$PWD/core

physics.subdir  = $$PWD/physics
physics.depends = core

orm.subdir = $$PWD/orm
orm.depends = core

!android:!ios { tools/gxgen.depends = orm }

DISTFILES += \
    README.md \
    BACKERS.md \
    LICENSE \
    LICENSING.md \
    CHANGELOG.md \
    CONTRIBUTING.md \
    CODE_OF_CONDUCT.md \
    SUPPORT.md \
    SECURITY.md \
    LICENSES/LicenseRef-KooijmanInc-Commercial.txt \
    LICENSES/GPL-3.0-only.txt \
    mkspecs/features/genesisx_app_core.prf \
    mkspecs/features/genesisx_assets.prf \
    mkspecs/features/gx_app_ab.prf \
    mkspecs/features/gx_app_analytics.prf \
    mkspecs/features/gx_app_audiorecorder.prf \
    mkspecs/features/gx_app_auth.prf \
    mkspecs/features/gx_app_background.prf \
    mkspecs/features/gx_app_billing.prf \
    mkspecs/features/gx_app_biometrics.prf \
    mkspecs/features/gx_app_camera.prf \
    mkspecs/features/gx_app_cast.prf \
    mkspecs/features/gx_app_clipboard.prf \
    mkspecs/features/gx_app_config.prf \
    mkspecs/features/gx_app_connectivity.prf \
    mkspecs/features/gx_app_contacts.prf \
    mkspecs/features/gx_app_crash.prf \
    mkspecs/features/gx_app_deeplinks.prf \
    mkspecs/features/gx_app_files.prf \
    mkspecs/features/gx_app_haptics.prf \
    mkspecs/features/gx_app_imagepicker.prf \
    mkspecs/features/gx_app_intents.prf \
    mkspecs/features/gx_app_location.prf \
    mkspecs/features/gx_app_logging.prf \
    mkspecs/features/gx_app_media.prf \
    mkspecs/features/gx_app_notifications.prf \
    mkspecs/features/gx_app_permissions.prf \
    mkspecs/features/gx_app_remoteconfig.prf \
    mkspecs/features/gx_app_review.prf \
    mkspecs/features/gx_app_securestore.prf \
    mkspecs/features/gx_app_sensors.prf \
    mkspecs/features/gx_app_share.prf \
    mkspecs/features/gx_app_updater.prf \
    mkspecs/gx_app_calendar.prf \
    mkspecs/modules/qt_lib_genesisx_app_ab.pri \
    mkspecs/modules/qt_lib_genesisx_app_analytics.pri \
    mkspecs/modules/qt_lib_genesisx_app_audiorecorder.pri \
    mkspecs/modules/qt_lib_genesisx_app_auth.pri \
    mkspecs/modules/qt_lib_genesisx_app_background.pri \
    mkspecs/modules/qt_lib_genesisx_app_billing.pri \
    mkspecs/modules/qt_lib_genesisx_app_biometrics.pri \
    mkspecs/modules/qt_lib_genesisx_app_calendar.pri \
    mkspecs/modules/qt_lib_genesisx_app_camera.pri \
    mkspecs/modules/qt_lib_genesisx_app_cast.pri \
    mkspecs/modules/qt_lib_genesisx_app_clipboard.pri \
    mkspecs/modules/qt_lib_genesisx_app_config.pri \
    mkspecs/modules/qt_lib_genesisx_app_connectivity.pri \
    mkspecs/modules/qt_lib_genesisx_app_contacts.pri \
    mkspecs/modules/qt_lib_genesisx_app_core.pri \
    mkspecs/modules/qt_lib_genesisx_app_crash.pri \
    mkspecs/modules/qt_lib_genesisx_app_deeplinks.pri \
    mkspecs/modules/qt_lib_genesisx_app_files.pri \
    mkspecs/modules/qt_lib_genesisx_app_haptics.pri \
    mkspecs/modules/qt_lib_genesisx_app_imagepicker.pri \
    mkspecs/modules/qt_lib_genesisx_app_intents.pri \
    mkspecs/modules/qt_lib_genesisx_app_location.pri \
    mkspecs/modules/qt_lib_genesisx_app_logging.pri \
    mkspecs/modules/qt_lib_genesisx_app_media.pri \
    mkspecs/modules/qt_lib_genesisx_app_notifications.pri \
    mkspecs/modules/qt_lib_genesisx_app_permissions.pri \
    mkspecs/modules/qt_lib_genesisx_app_preferences.pri \
    mkspecs/modules/qt_lib_genesisx_app_remoteconfig.pri \
    mkspecs/modules/qt_lib_genesisx_app_review.pri \
    mkspecs/modules/qt_lib_genesisx_app_securestore.pri \
    mkspecs/modules/qt_lib_genesisx_app_sensors.pri \
    mkspecs/modules/qt_lib_genesisx_app_share.pri \
    mkspecs/modules/qt_lib_genesisx_app_updater.pri \
    mkspecs/modules/qt_lib_genesisx_assets.pri \
    scripts/install.bat \
    scripts/install.sh \
    tools/qtcreator-wizard/install-wizard.sh


SUBDIRS += zdev

# build physics too:
# qmake genesisx.pro "CONFIG+=genesisx_build_physics"

