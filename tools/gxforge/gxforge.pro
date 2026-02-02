# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

TEMPLATE = app
TARGET = forge
QT += core qml quick widgets
CONFIG += console c++23

win32 {
    VERSION = 1.0.0
    ICON += ../../core/resources/logo.ico
    RC_ICONS += ../../core/resources/logo.ico

    CONFIG += embed_manifest_exe
}

include(../../common/qmake-target-platform.pri)
include(../../common/qmake-destination-path.pri)

GX_CFG = debug
CONFIG(release, debug|release): GX_CFG = release
CONFIG(profile, debug|release|profile): GX_CFG = profile

GENESISX_BUILD_ROOT = $$clean_path($$PWD/../..)
GENESISX_LIBDIR = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/$$GX_CFG

SOURCES += \
    src/main.cpp \
    src/utils/GXGltfConverter.cpp

INCLUDEPATH += \
    src \
    ../../gx3d/include
DEPENDPATH += \
    ../../gx3d/include

LIBS += -L$$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/$$GX_CFG -lgenesisx_3d -lgenesisx

win32 {
    PRE_TARGETDEPS += $$GENESISX_LIBDIR/libgenesisx_3d.a \
                      $$GENESISX_LIBDIR/libgenesisx.a
} else {
    # Unix (Linux + macOS): depend on the *shared* libs (so/dylib) instead of .a
    GENESISX_ORM_SHLIB = $$GENESISX_LIBDIR/$$QMAKE_PREFIX_SHLIBgenesisx_3d.$$QMAKE_EXTENSION_SHLIB
    GENESISX_SHLIB     = $$GENESISX_LIBDIR/$$QMAKE_PREFIX_SHLIBgenesisx.$$QMAKE_EXTENSION_SHLIB

    # message(get correct lib is $$GENESISX_LIB)
    PRE_TARGETDEPS += $$GENESISX_LIBDIR/libgenesisx_3d.$$QMAKE_EXTENSION_SHLIB
                      $$GENESISX_LIBDIR/libgenesisx.$$QMAKE_EXTENSION_SHLIB
}

DESTDIR = $$PWD/bin

win32 {
    GX_DLL_3D = $$shell_path($$GENESISX_LIBDIR/genesisx_3d.dll)
    GX_DLL_3D_DEST = $$shell_path($$DESTDIR/genesisx_3d.dll)
    GX_A_3D = $$shell_path($$GENESISX_LIBDIR/libgenesisx_3d.a)
    GX_A_3D_DEST = $$shell_path($$DESTDIR/libgenesisx_3d.a)

    exists($$GX_DLL_3D) {
        QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_DLL_3D) $$shell_quote($$GX_DLL_3D_DEST)
        message([genesisx_3d] 3d library .dll file copied to $$DEST_LIBDIRS)
    }
    exists($$GX_A_3D) {
        QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_A_3D) $$shell_quote($$GX_A_3D_DEST)
        message([genesisx_3d] 3d library $$GX_A_3D $$GX_3D_A file copied to $$DEST_LIBDIRS)
    }

    GX_DLL = $$shell_path($$GENESISX_LIBDIR/genesisx.dll)
    GX_DLL_DEST = $$shell_path($$DESTDIR/genesisx.dll)
    GX_A = $$shell_path($$GENESISX_LIBDIR/libgenesisx.a)
    GX_A_DEST = $$shell_path($$DESTDIR/libgenesisx.a)

    exists($$GX_DLL) {
        QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_DLL) $$shell_quote($$GX_DLL_DEST)
        message([genesisx] library .dll file copied to $$DEST_LIBDIRS)
    }
    exists($$GX_A) {
        QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_A) $$shell_quote($$GX_A_DEST)
        message([genesisx] library .a file copied to $$DEST_LIBDIRS)
    }
}

QML_IMPORT_PATH += $$PWD
QML2_IMPORT_PATH += $$PWD

RESOURCES += \
    ../../core/resources/core.qrc \
    components.qrc \
    views.qrc

HEADERS += \
    src/utils/GXGltfConverter.h
