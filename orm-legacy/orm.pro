# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

QT += network sql qml
TEMPLATE = lib
TARGET = genesisx_orm
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
    DEFINES += GENESISX_ORM_STATIC
} else {
    DEFINES += GENESISX_ORM_LIBRARY
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

# ORM links to core; search same central dir
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

INCLUDEPATH += $$GENESISX_BUILD_ROOT/orm/include

HEADERS += $$files($$PWD/include/GenesisX/*.h, true) \
    $$files($$PWD/src/*.h, true)

SOURCES += $$files($$PWD/src/*.cpp, true)
