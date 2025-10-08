# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

QT -= gui
QT += core qml widgets
linux:!android: QT += dbus
TEMPLATE = lib
TARGET = genesisx
CONFIG += c++23

ios {
    CONFIG -= dll shared
    CONFIG += staticlib
} else {
    CONFIG += shared
}

DEFINES += GENESISX_CORE_BUILD
DEFINES += GENESISX_HAS_NOTIFICATIONS
DEFINES += GX_ENABLE_STARTUP_AUTO_REGISTER

GENESISX_BUILD_ROOT = $$clean_path($$PWD/..)
include($$GENESISX_BUILD_ROOT/common/qmake-target-platform.pri)
include($$GENESISX_BUILD_ROOT/common/qmake-destination-path.pri)
include($$GENESISX_BUILD_ROOT/3rdparty/firebase/firebase.pri)

GX_CFG = debug
CONFIG(release, debug|release): GX_CFG = release
CONFIG(profile, debug|release|profile): GX_CFG = profile

# Central output dir
DESTDIR = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/$$GX_CFG
message([core] DESTDIR = $$DESTDIR)

# Put MinGW import lib next to the DLL (otherwise it lands in ./debug)
win32-g++: QMAKE_LFLAGS_SHLIB += -Wl,--out-implib,$$DESTDIR/lib$${TARGET}.a

INCLUDEPATH += $$GENESISX_BUILD_ROOT/core/include

HEADERS += $$files($$PWD/include/GenesisX/core/*.h) \
    $$PWD/include/GenesisX/NotificationsQml.h \
    $$PWD/include/GenesisX/genesisx_global.h \
    $$PWD/src/notifications/NotificationHandler.h \
    $$PWD/src/notifications/fcm_android.h
SOURCES += $$files($$PWD/src/*.cpp) \
    $$PWD/src/notifications/NotificationHandler.cpp \
    $$PWD/src/notifications/NotificationsQml.cpp \
    $$PWD/src/notifications/fcm_android.cpp

# Optional install
# headers.path = $$[QT_INSTALL_PREFIX]/include/GenesisX/core
# headers.files = $$HEADERS
# target.path  = $$[QT_INSTALL_PREFIX]/lib
# INSTALLS += headers target

DISTFILES += \
    # $$PWD/common/genesisx_core.pri \
    $$PWD/android-template/gradle.properties \
    $$PWD/android-template/gradle.properties.in \
    $$PWD/android-template/build.gradle \
    $$PWD/android-template/google-services.json \
    $$GENESISX_BUILD_ROOT/3rdparty/firebase_cpp_sdk/Android/firebase_dependencies.gradle

RESOURCES += \
    $$PWD/resources/core.qrc

# --- Define values BEFORE QMAKE_SUBSTITUTES runs ---
GENESISX_ROOT_ABS            = $$clean_path($$PWD/..)
FIREBASE_CPP_SDK_DIR         = $$clean_path($$GENESISX_ROOT_ABS/3rdparty/firebase_cpp_sdk)
FIREBASE_CPP_SDK_DIR        ~= s,\\\\,/,g
FIREBASE_DEPENDENCIES_GRADLE = $$FIREBASE_CPP_SDK_DIR/Android/firebase_dependencies.gradle

# Make sure these are exported (visible to any nested parsing step)
export(FIREBASE_CPP_SDK_DIR)
export(FIREBASE_DEPENDENCIES_GRADLE)

# Input template and expected generated output path in the SHADOW dir
ANDROID_TPL_SRC_DIR = $$clean_path($$PWD/android-template)
GRADLE_PROPS_IN     = $$ANDROID_TPL_SRC_DIR/gradle.properties.in
GRADLE_PROPS_GEN    = $$shadowed($$ANDROID_TPL_SRC_DIR)/gradle.properties

# Tell qmake to generate GRADLE_PROPS_GEN from GRADLE_PROPS_IN
QMAKE_SUBSTITUTES += $$GRADLE_PROPS_IN

# (Optional) also mark the template as an "other" file so Creator shows it
OTHER_FILES += $$GRADLE_PROPS_IN

# --- Loud diagnostics at qmake time ---
message([substitute] GENESISX_ROOT_ABS            = $$GENESISX_ROOT_ABS)
message([substitute] FIREBASE_CPP_SDK_DIR         = $$FIREBASE_CPP_SDK_DIR)
message([substitute] FIREBASE_DEPENDENCIES_GRADLE = $$FIREBASE_DEPENDENCIES_GRADLE)
message([substitute] IN  = $$GRADLE_PROPS_IN)
message([substitute] OUT = $$GRADLE_PROPS_GEN)

# Show whether qmake sees the input template
exists($$GRADLE_PROPS_IN) {
    message([substitute] template EXISTS)
} else {
    message([substitute] template MISSING)
}

GRADLE_PROPS_DST = $$ANDROID_TPL_SRC_DIR/gradle.properties
QMAKE_POST_LINK += $$QMAKE_COPY $$shell_path($$GRADLE_PROPS_GEN) $$shell_path($$GRADLE_PROPS_DST)

