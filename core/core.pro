# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

QT += core qml quick widgets network
linux:!android: QT += dbus
TEMPLATE = lib
TARGET = genesisx
CONFIG += c++23

ios {
    QMAKE_MAC_XCODE_SETTINGS += ALWAYS_SEARCH_USER_PATHS=NO
    QMAKE_MAC_XCODE_SETTINGS += USE_HEADERMAP=YES
    QMAKE_MAC_XCODE_SETTINGS += SEPARATE_HEADERMAP=YES
    CONFIG -= dll shared
    CONFIG += staticlib
    LIBS += -framework UserNotifications
    LIBS += -framework UIKit
    LIBS += -framework Foundation
    QMAKE_OBJECTIVE_CFLAGS += -fobjc-arc
    QMAKE_OBJECTIVE_CXXFLAGS += -fobjc-arc

    QMAKE_LFLAGS += -ObjC
    OBJECTIVE_SOURCES += \
        $$PWD/src/app/Notifications/NotificationHandler_apple.mm \
        $$PWD/src/app/Notifications/GXAppDelegate+Push_ios.mm \
        $$PWD/src/app/Background/BackgroundAudio.mm
} else:macos {
    QMAKE_MAC_XCODE_SETTINGS += ALWAYS_SEARCH_USER_PATHS=NO
    QMAKE_MAC_XCODE_SETTINGS += USE_HEADERMAP=YES
    QMAKE_MAC_XCODE_SETTINGS += SEPARATE_HEADERMAP=YES
    CONFIG += shared staticlib
    LIBS += -framework UserNotifications
    LIBS += -framework AppKit
    LIBS += -framework Cocoa
    LIBS += -framework Foundation
    QMAKE_LFLAGS += -ObjC
    QMAKE_OBJECTIVE_CFLAGS += -fobjc-arc
    QMAKE_OBJECTIVE_CXXFLAGS += -fobjc-arc
    QMAKE_LFLAGS += -Wl,-undefined,dynamic_lookup
    QMAKE_MAC_XCODE_SETTINGS += OTHER_LDFLAGS="$(inherited) -ObjC"
    OBJECTIVE_SOURCES += \
        $$PWD/src/app/Notifications/NotificationHandler_apple.mm \
        $$PWD/src/app/Notifications/GXPushBridge.mm
        # $$PWD/src/app/Background/BackgroundAudio.mm
#} else:win32 {
#    CONFIG += dll
} else:wasm {
    CONFIG -= shared dll plugin
    CONFIG += staticlib
} else {
    CONFIG += shared
}

contains(CONFIG, staticlib) {
    DEFINES += GENESISX_CORE_STATIC
} else {
    DEFINES += GENESISX_CORE_LIBRARY
}
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

HEADERS += \
    $$files($$PWD/include/GenesisX/*.h, true) \
    $$files($$PWD/src/*.h, true)
SOURCES += \
    $$files($$PWD/src/*.cpp, true)

android {
    SOURCES += \
        # $$files($$PWD/src/*Android.cpp, true)
} else {
    HEADERS -= \
        $$PWD/include/GenesisX/Auth/Auth.h \
        $$PWD/include/GenesisX/Background/gx_background_audio.h \
        $$PWD/include/GenesisX/Biometrics/Biometrics.h \
        $$PWD/src/app/Biometrics/BiometricsQml.h

    SOURCES -= \
        $$files($$PWD/src/*Android.cpp, true) \
        $$PWD/src/app/Auth/Auth.cpp \
        $$PWD/src/app/Background/gx_audio_service.cpp \
        $$PWD/src/app/Background/BackgroundBridge_jni.cpp \
        $$PWD/src/app/Biometrics/BiometricsAndroid.cpp \
        $$PWD/src/app/Biometrics/Biometrics.cpp \
        $$PWD/src/app/Biometrics/BiometricsQml.cpp
}


# Optional install
# headers.path = $$[QT_INSTALL_PREFIX]/include/GenesisX/core
# headers.files = $$HEADERS
# target.path  = $$[QT_INSTALL_PREFIX]/lib
# INSTALLS += headers target

QML_IMPORT_PATH += $$PWD/qml

DISTFILES += \
    $$files($$PWD/qml/*, true) \
    $$files($$PWD/android-template/*, true) \
    $$files($$PWD/apple-template/*, true) \
    $$GENESISX_BUILD_ROOT/3rdparty/firebase_cpp_sdk/Android/firebase_dependencies.gradle \
    qml/GenesisX/App/Biometrics/qmldir \
    qml/GenesisX/App/Permissions/permissions.qmltypes \
    qml/GenesisX/App/Permissions/qmldir \
    qml/GenesisX/Core/Navigation/qmldir \
    qml/GenesisX/Core/SystemInfo/qmldir \
    qml/GenesisX/Core/SystemInfo/systeminfo.qmltypes \
    qml/GenesisX/Atoms/qmldir \
    qml/GenesisX/Atoms/GxRaisedButton.qml \
    src/app/Background/android/src/com/genesisx/background/GXAudioService.kt \
    src/app/Background/android/src/com/genesisx/background/GXBackgroundBridge.kt \
    src/app/Background/android/src/com/genesisx/background/GXMediaCmdReceiver.kt \
    src/app/Biometrics/android/src/main/java/biometrics/GxBiometrics.java \
    src/app/Biometrics/android/src/main/java/org/qtproject/qt/android/QtActivityUtils.java \
    src/app/Cast/android/src/main/java/com/genesisx/cast/GXCastBridge.java \
    src/app/Cast/android/src/main/java/com/genesisx/cast/GXCastChooserActivity.java \
    src/app/Cast/android/src/main/java/com/genesisx/cast/GXCastKeepAliveService.java \
    src/app/Cast/android/src/main/java/com/genesisx/cast/GXCastManager.java \
    src/app/Cast/android/src/main/java/com/genesisx/cast/GXCastOptionsProvider.java \
    src/app/Permissions/android/src/main/java/permissions/GxPermissions.java

RESOURCES += \
    $$PWD/resources/core.qrc \
    qml/core_modules.qrc

# --- Define values BEFORE QMAKE_SUBSTITUTES runs ---
GENESISX_ROOT_ABS            = $$clean_path($$PWD/..)
FIREBASE_CPP_SDK_DIR         = $$clean_path($$GENESISX_ROOT_ABS/3rdparty/firebase_cpp_sdk)
FIREBASE_CPP_SDK_DIR        ~= s,\\\\,/,g
FIREBASE_DEPENDENCIES_GRADLE = $$FIREBASE_CPP_SDK_DIR/Android/firebase_dependencies.gradle

# Make sure these are exported (visible to any nested parsing step)
export(FIREBASE_CPP_SDK_DIR)
export(FIREBASE_DEPENDENCIES_GRADLE)

android {
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
QMAKE_POST_LINK += $$escape_expand(\\n\\t)$$QMAKE_COPY \
                   $$shell_path($$GRADLE_PROPS_GEN) \
                   $$shell_path($$GRADLE_PROPS_DST)
}
