# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

TEMPLATE = app
TARGET = gxgen
QT += core sql
CONFIG += console c++23

include(../../common/qmake-target-platform.pri)
include(../../common/qmake-destination-path.pri)

GX_CFG = debug
CONFIG(release, debug|release): GX_CFG = release
CONFIG(profile, debug|release|profile): GX_CFG = profile

GENESISX_BUILD_ROOT = $$clean_path($$PWD/../..)
GENESISX_LIBDIR = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/$$GX_CFG

SOURCES += \
    main.cpp

INCLUDEPATH += ../../orm/include
DEPENDPATH += ../../orm/include

LIBS += -L$$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/$$GX_CFG -lgenesisx_orm -lgenesisx

PRE_TARGETDEPS += $$GENESISX_LIBDIR/libgenesisx_orm.a \
                  $$GENESISX_LIBDIR/libgenesisx.a

DESTDIR = $$PWD/$$GX_CFG

win32 {
    GX_DLL_ORM = $$shell_path($$GENESISX_LIBDIR/genesisx_orm.dll)
    GX_DLL_ORM_DEST = $$shell_path($$DESTDIR/genesisx_orm.dll)
    GX_A_ORM = $$shell_path($$GENESISX_LIBDIR/libgenesisx_orm.a)
    GX_A_ORM_DEST = $$shell_path($$DESTDIR/libgenesisx_orm.a)

    exists($$GX_DLL_ORM) {
        QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_DLL_ORM) $$shell_quote($$GX_DLL_ORM_DEST)
        message([genesisx_orm] orm library .dll file copied to $$DEST_LIBDIRS)
    }
    exists($$GX_A_ORM) {
        QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_A_ORM) $$shell_quote($$GX_A_ORM_DEST)
        message([genesisx_orm] orm library $$GX_A_ORM $$GX_ORM_A file copied to $$DEST_LIBDIRS)
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

# DESTDIR = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/$$GX_CFG
