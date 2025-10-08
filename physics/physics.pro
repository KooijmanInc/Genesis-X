# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

QT += core

TEMPLATE = lib
TARGET = genesisx_physics
CONFIG += c++23

ios {
    CONFIG -= dll shared
    CONFIG += staticlib
} else {
    CONFIG += shared
}

DEFINES += GENESISX_PHYSICS_BUILD

GENESISX_BUILD_ROOT = $$clean_path($$PWD/..)
include($$GENESISX_BUILD_ROOT/common/qmake-target-platform.pri)
include($$GENESISX_BUILD_ROOT/common/qmake-destination-path.pri)

GX_CFG = debug
CONFIG(release, debug|release): GX_CFG = release
CONFIG(profile, debug|release|profile): GX_CFG = profile

DESTDIR = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/$$GX_CFG
message([physics] DESTDIR = $$DESTDIR)

# Put import lib in the same central bin dir
win32-g++: QMAKE_LFLAGS_SHLIB += -Wl,--out-implib,$$DESTDIR/lib$${TARGET}.a

# Physics links to core; search same central dir
QMAKE_LIBDIR += $$DESTDIR
android {
    LIBS += -lgenesisx_arm64-v8a
} else {
    LIBS += -lgenesisx
}

INCLUDEPATH += $$GENESISX_BUILD_ROOT/physics/include/GenesisX

HEADERS += $$files($$PWD/include/GenesisX/physics/*.h)
SOURCES += $$files($$PWD/src/*.cpp)

# QMAKE_LIBDIR += $$GENESISX_OUT_BIN
# LIBS += -lgenesisx   # depends on core

# LIBS += -L$$OUT_PWD/../core -lgenesisx
# ios:LIBS += $$OUT_PWD/../core/libgenesisx.a
# CONFIG(debug, debug|release) {
#     LIBS += -L$$OUT_PWD/../core/debug
#     PRE_TARGETDEPS += $$OUT_PWD/../core/debug/libgenesisx.a
# } else {
#     LIBS += -L$$OUT_PWD/../core/release
#     PRE_TARGETDEPS += $$OUT_PWD/../core/release/libgenesisx.a
# }
# LIBS += -lgenesisx

DISTFILES += \
    common/genesisx_physics.pri \
    physics.json

