# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

# ---- GenesisX Physics qmake module ----
# mkspecs/modules/qt_lib_genesisx_physics.pri

!defined(__GX_PHYS_PRI_INCLUDED__, var) {
    load(gx_app_root)
    __GX_PHYS_PRI_INCLUDED__ = 1
    message([genesisx_physics] module loaded from $$PWD)

    GENESISX_BUILD_ROOT = $$clean_path($$PWD/../..)
    include($$GENESISX_BUILD_ROOT/common/qmake-target-platform.pri)
    include($$GENESISX_BUILD_ROOT/common/qmake-destination-path.pri)

    QT.genesisx_physics.name          = genesisx_physics
    QT.genesisx_physics.depends       = core gui
    QT.genesisx_physics.friendly_name = GenesisX (Physics)

    INCLUDEPATH += $$clean_path($$GENESISX_BUILD_ROOT/physics/include)

    phys.libdir.debug   = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/debug
    phys.libdir.release = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/release
    phys.libdir.profile = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/profile

    core.libdir.debug   = $$phys.libdir.debug
    core.libdir.release = $$phys.libdir.release
    core.libdir.profile = $$phys.libdir.profile

    QMAKE_LIBDIR += $$phys.libdir.debug $$phys.libdir.release $$phys.libdir.profile \
                    $$core.libdir.debug $$core.libdir.release $$core.libdir.profile

    GX_ANDROID =
    android: GX_ANDROID = 1
    !isEmpty(ANDROID_PLATFORM): GX_ANDROID = 1
    contains(QMAKE_XSPEC, android): GX_ANDROID = 1
    contains(QMAKE_SPEC, android):  GX_ANDROID = 1
    contains(QMAKESPEC, android):   GX_ANDROID = 1
    contains($$[QT_INSTALL_PREFIX], /android_)|contains($$[QT_INSTALL_PREFIX], \\android_) { GX_ANDROID = 1 }

    GX_ABI = $$QT_ARCH
    isEmpty(GX_ABI): GX_ABI = arm64-v8a
    contains(GX_ABI, arm64): GX_ABI = arm64-v8a

    !isEmpty(GX_ANDROID) {
        LIBS += -lgenesisx_physics_$${GX_ABI} -lgenesisx_$${GX_ABI}
    } else:contains(QMAKE_HOST.os, Linux) {
        LIBS += -lgenesisx_physics -lgenesisx
    } else {
        LIBS += -lgenesisx_physics -lgenesisx
        # msvc:CONFIG(debug,debug|release): LIBS += -lGenesisXPhysicsd -lGenesisXd
    }

    contains(PLATFORM_PATH, windows) {
        # Base directory where DLLs are built
        GX_LIB_BASE = $$GENESISX_BUILD_ROOT/bin/windows/$$COMPILER_PATH/$$PROCESSOR_PATH

        # Target DLLs for all build types
        GX_DLL_DEBUG    = $$shell_path($$GX_LIB_BASE/debug/genesisx.dll)
        GX_DLL_RELEASE  = $$shell_path($$GX_LIB_BASE/release/genesisx.dll)
        GX_DLL_PROFILE  = $$shell_path($$GX_LIB_BASE/profile/genesisx.dll)

        GX_PHYS_DLL_DEBUG    = $$shell_path($$GX_LIB_BASE/debug/genesisx_physics.dll)
        GX_PHYS_DLL_RELEASE  = $$shell_path($$GX_LIB_BASE/release/genesisx_physics.dll)
        GX_PHYS_DLL_PROFILE  = $$shell_path($$GX_LIB_BASE/profile/genesisx_physics.dll)

        # To DLLs for all build types
        GX_DLL_DEST_DEBUG    = $$shell_path($$ANDROID_PACKAGE_SOURCE_DIR/binaries/$$LOCAL_DESTINATION_PATH/debug/genesisx.dll)
        GX_DLL_DEST_RELEASE  = $$shell_path($$ANDROID_PACKAGE_SOURCE_DIR/binaries/$$LOCAL_DESTINATION_PATH/release/genesisx.dll)
        GX_DLL_DEST_PROFILE  = $$shell_path($$ANDROID_PACKAGE_SOURCE_DIR/binaries/$$LOCAL_DESTINATION_PATH/profile/genesisx.dll)

        GX_PHYS_DLL_DEST_DEBUG    = $$shell_path($$ANDROID_PACKAGE_SOURCE_DIR/binaries/$$LOCAL_DESTINATION_PATH/debug/genesisx_physics.dll)
        GX_PHYS_DLL_DEST_RELEASE  = $$shell_path($$ANDROID_PACKAGE_SOURCE_DIR/binaries/$$LOCAL_DESTINATION_PATH/release/genesisx_physics.dll)
        GX_PHYS_DLL_DEST_PROFILE  = $$shell_path($$ANDROID_PACKAGE_SOURCE_DIR/binaries/$$LOCAL_DESTINATION_PATH/profile/genesisx_physics.dll)

        # Post-link copy (only copy those that exist)
        exists($$GX_DLL_DEBUG): QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_DLL_DEBUG) $$shell_quote($$GX_DLL_DEST_DEBUG)
        exists($$GX_DLL_RELEASE): QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_DLL_RELEASE) $$shell_quote($$GX_DLL_DEST_RELEASE)
        exists($$GX_DLL_PROFILE): QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_DLL_PROFILE) $$shell_quote($$GX_DLL_DEST_PROFILE)

        exists($$GX_PHYS_DLL_DEBUG): QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_PHYS_DLL_DEBUG) $$shell_quote($$GX_PHYS_DLL_DEST_DEBUG)
        exists($$GX_PHYS_DLL_RELEASE): QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_PHYS_DLL_RELEASE) $$shell_quote($$GX_PHYS_DLL_DEST_RELEASE)
        exists($$GX_PHYS_DLL_PROFILE): QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_PHYS_DLL_PROFILE) $$shell_quote($$GX_PHYS_DLL_DEST_PROFILE)
    }

    contains(QMAKE_HOST.os, Linux) {
        # Where Genesis-X builds its libs
        GX_LIB_BASE = $$clean_path($$GENESISX_BUILD_ROOT/bin/linux/$$COMPILER_PATH/$$PROCESSOR_PATH)
        GX_LIB_BASE = $$replace(GX_LIB_BASE, \\\\, /)

        # Where your app binaries live
        GX_APP_BASE = $$clean_path($$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH)
        GX_APP_BASE = $$replace(GX_APP_BASE, \\\\, /)

        # Make the runtime loader search the exe directory
        QMAKE_LFLAGS += -Wl,-rpath,\$$ORIGIN

        builds = debug
        builds += release
        builds += profile
        for (build, builds) {
            SRC_DIR = $$GX_LIB_BASE/$$build
            DST_DIR = $$GX_APP_BASE/$$build
            # Expand the actual files *now* (qmake time). This returns only existing files.
            SO_LIST = $$files($$SRC_DIR/libgenesisx.so*)
            SO_LIST += $$files($$SRC_DIR/libgenesisx_physics.so*)

            message([genesisx] Linux copy from $$SRC_DIR -> $$DST_DIR)
            message([genesisx] Linux libs: $$SO_LIST)

            # Ensure destination exists
            QMAKE_POST_LINK += $$escape_expand(\\n\\t)mkdir -p $$shell_quote($$DST_DIR)

            # Emit one cp command per file (no shell loops; robust in Makefiles)
            for(f, SO_LIST) {
                QMAKE_POST_LINK += $$escape_expand(\\n\\t)cp -f $$shell_quote($$f) $$shell_quote($$DST_DIR)/
            }
        }
    }

    contains(QMAKE_HOST.os, Darwin) {
        #LIBS += -framework UserNotifications
        #ios: LIBS += -framework UIKit
        #macos: LIBS += -framework AppKit UIKit Cocoa
        # Where Genesis-X builds its libs
        # QMAKE_MAC_XCODE_SETTINGS += OTHER_LDFLAGS="$(inherited) -ObjC"
        GX_LIB_BASE = $$clean_path($$GENESISX_BUILD_ROOT/bin/osx/$$COMPILER_PATH/$$PROCESSOR_PATH)
        GX_LIB_BASE = $$replace(GX_LIB_BASE, \\\\, /)

        # Where your app binaries live
        GX_APP_BASE = $$clean_path($$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH)
        GX_APP_BASE = $$replace(GX_APP_BASE, \\\\, /)

        # Make the runtime loader search the exe directory
        # QMAKE_LFLAGS += -Wl,-rpath,\$$ORIGIN

        builds = debug
        builds += release
        builds += profile
        for (build, builds) {
            SRC_DIR = $$GX_LIB_BASE/$$build
            DST_DIR = $$GX_APP_BASE/$$build
            # Expand the actual files *now* (qmake time). This returns only existing files.
            SO_LIST = $$files($$SRC_DIR/libgenesisx*)
            SO_LIST += $$files($$SRC_DIR/libgenesisx_physics*)

            # message([genesisx] Linux copy from $$SRC_DIR -> $$DST_DIR)
            # message([genesisx] Linux libs: $$SO_LIST)

            # Ensure destination exists
            QMAKE_POST_LINK += $$escape_expand(\\n\\t)mkdir -p $$shell_quote($$DST_DIR)

            # Emit one cp command per file (no shell loops; robust in Makefiles)
            for(f, SO_LIST) {
                # exists($$f) {
                    QMAKE_POST_LINK += $$escape_expand(\\n\\t)cp -f $$shell_quote($$f) $$shell_quote($$DST_DIR)/
                # }
            }
        }
    }

    linux:!android {
        # Embed a runpath that points to the exe directory at runtime
        QMAKE_LFLAGS += -Wl,-rpath,\$$ORIGIN
        # (Optional) also add specific config dirs if you want:
        QMAKE_RPATHDIR += $$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/debug
        QMAKE_RPATHDIR += $$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/release
        QMAKE_RPATHDIR += $$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/profile
    }

    contains(QMAKE_HOST.os, Darwin) {
        # Embed a runpath that points to the exe directory at runtime
        # QMAKE_LFLAGS += -Wl,-rpath,\$$ORIGIN
        # (Optional) also add specific config dirs if you want:
        # QMAKE_RPATHDIR += $$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/debug
        #QMAKE_RPATHDIR += $$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/release
        #QMAKE_RPATHDIR += $$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/profile
    }

    # contains(QMAKE_HOST.os, Linux) {
    #     # Base directory where DLLs are built
    #     GX_LIB_BASE = $$GENESISX_BUILD_ROOT/bin/linux/$$COMPILER_PATH/$$PROCESSOR_PATH


    #     # Target DLLs for all build types
    #     GX_DLL_DEBUG    = $$shell_path($$GX_LIB_BASE/debug/libgenesisx.so)
    #     GX_DLL_RELEASE  = $$shell_path($$GX_LIB_BASE/release/libgenesisx.so)
    #     GX_DLL_PROFILE  = $$shell_path($$GX_LIB_BASE/profile/libgenesisx.so)

    #     GX_PHYS_DLL_DEBUG    = $$shell_path($$GX_LIB_BASE/debug/libgenesisx_physics.so)
    #     GX_PHYS_DLL_RELEASE  = $$shell_path($$GX_LIB_BASE/release/libgenesisx_physics.so)
    #     GX_PHYS_DLL_PROFILE  = $$shell_path($$GX_LIB_BASE/profile/libgenesisx_physics.so)

    #     # To DLLs for all build types
    #     GX_DLL_DEST_DEBUG    = $$shell_path($$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/debug/libgenesisx.so)
    #     GX_DLL_DEST_RELEASE  = $$shell_path($$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/release/libgenesisx.so)
    #     GX_DLL_DEST_PROFILE  = $$shell_path($$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/profile/libgenesisx.so)

    #     GX_PHYS_DLL_DEST_DEBUG    = $$shell_path($$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/debug/libgenesisx_physics.so)
    #     GX_PHYS_DLL_DEST_RELEASE  = $$shell_path($$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/release/libgenesisx_physics.so)
    #     GX_PHYS_DLL_DEST_PROFILE  = $$shell_path($$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/profile/libgenesisx_physics.so)

    #     # Post-link copy (only copy those that exist)
    #     exists($$GX_DLL_DEBUG): QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_DLL_DEBUG) $$shell_quote($$GX_DLL_DEST_DEBUG)
    #     exists($$GX_DLL_RELEASE): QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_DLL_RELEASE) $$shell_quote($$GX_DLL_DEST_RELEASE)
    #     exists($$GX_DLL_PROFILE): QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_DLL_PROFILE) $$shell_quote($$GX_DLL_DEST_PROFILE)

    #     exists($$GX_PHYS_DLL_DEBUG): QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_PHYS_DLL_DEBUG) $$shell_quote($$GX_PHYS_DLL_DEST_DEBUG)
    #     exists($$GX_PHYS_DLL_RELEASE): QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_PHYS_DLL_RELEASE) $$shell_quote($$GX_PHYS_DLL_DEST_RELEASE)
    #     exists($$GX_PHYS_DLL_PROFILE): QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_PHYS_DLL_PROFILE) $$shell_quote($$GX_PHYS_DLL_DEST_PROFILE)
    # }


    # ---- ANDROID copies ----
    !isEmpty(GX_ANDROID) {
        # GX_ABI = $$QT_ARCH
        # isEmpty(GX_ABI): GX_ABI = arm64-v8a

        GX_DIRS = \
            $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/debug \
            $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/release \
            $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/profile

        for(dir, GX_DIRS) {
            PHYS_SO = $$dir/libgenesisx_physics_$${GX_ABI}.so
            CORE_SO = $$dir/libgenesisx_$${GX_ABI}.so
            exists($$PHYS_SO) {
                ANDROID_EXTRA_LIBS += $$PHYS_SO
                message([genesisx_physics] add extra library ANDROID_EXTRA_LIBS += $$PHYS_SO)
            }
            exists($$CORE_SO) {
                ANDROID_EXTRA_LIBS += $$CORE_SO
                message([genesisx_physics] add extra library ANDROID_EXTRA_LIBS += $$CORE_SO)
            }
        }
    }

    message([genesisx_physics] libdirs = $$phys.libdir.debug | $$phys.libdir.release | $$phys.libdir.profile | BUILD_PATH = $$BUILD_PATH)
}
