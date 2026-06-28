# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

QT += core quick
TEMPLATE = lib
TARGET = genesisx_web
CONFIG += c++23

ios {
    CONFIG -= dll shared
    CONFIG += staticlib
} else:macos {
    CONFIG += shared staticlib
# } else:win32 {
# CONFIG += dll
} else:wasm {
    CONFIG -= shared dll plugin
    CONFIG += staticlib
} else {
    CONFIG += shared
}

contains(CONFIG, staticlib) {
    DEFINES += GENESISX_WEB_STATIC
} else {
    DEFINES += GENESISX_WEB_LIBRARY
}

GENESISX_BUILD_ROOT = $$clean_path($$PWD/..)
include($$GENESISX_BUILD_ROOT/common/qmake-target-platform.pri)
include($$GENESISX_BUILD_ROOT/common/qmake-destination-path.pri)

GX_CFG = debug
CONFIG(release, debug|release): GX_CFG = release
CONFIG(profile, debug|release|profile): GX_CFG = profile

DESTDIR = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/$$GX_CFG
message([orm] DESTDIR = $$DESTDIR)

# Put import lib in the same central bin dir
win32-g++: QMAKE_LFLAGS_SHLIB += -Wl,--out-implib,$$DESTDIR/lib$${TARGET}.a

# WEB links to core; search same central dir
QMAKE_LIBDIR += $$DESTDIR
android {
    contains(QT_ARCH, arm64-v8a) {
        LIBS += -lgenesisx_arm64-v8a
    } else: contains(QT_ARCH, x86_64) {
        LIBS += -lgenesisx_x86_64
    }
} else {
    LIBS += -lgenesisx
}

INCLUDEPATH += $$GENESISX_BUILD_ROOT/web/include

win32 {
    LIBS += -lole32 -loleaut32 -luuid
    INCLUDEPATH += $$GENESISX_BUILD_ROOT/3rdparty/webview2/include
}

HEADERS += \
    $$files($$PWD/include/GenesisX/*.h, true) \
    $$files($$PWD/src/*.h, true) \
    src/windows/GXComCallback.h

SOURCES += \
    $$files($$PWD/src/*.cpp, true)

QML_INPORT_PATH += $$PWD/qml

DISTFILES += \
    $$files($$PWD/qml/*, true)

RESOURCES += \
    qml/web_modules.qrc

