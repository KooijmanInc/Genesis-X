# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

# ---- GenesisX (core) qmake module ----
# mkspecs/modules/qt_lib_genesisx.pri

!defined(__GX_CORE_PRI_INCLUDED__, var) {
    # QMAKEFEATURES += D:/projects/qt/progs/learning/qtfeatures
    load(gx_app_root)
    # load(gx_app_root)
    __GX_CORE_PRI_INCLUDED__ = 1
    message([genesisx] module loaded from $$PWD)

    GENESISX_BUILD_ROOT = $$clean_path($$PWD/../..)
    include($$GENESISX_BUILD_ROOT/common/qmake-target-platform.pri)
    include($$GENESISX_BUILD_ROOT/common/qmake-destination-path.pri)

    QT.genesisx.name          = genesisx
    QT.genesisx.depends       = core
    QT.genesisx.friendly_name = GenesisX (Core)

    GX_CORE_INC_ROOT = $$clean_path($$GENESISX_BUILD_ROOT/core/include)
    !exists($$GX_CORE_INC_ROOT/GenesisX) { GX_CORE_INC_ROOT = $$clean_path($$GENESISX_BUILD_ROOT/include) }
    INCLUDEPATH += $$GX_CORE_INC_ROOT

    # Add all config libdirs in priority order (debug first)
    genesisx.libdir.debug   = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/debug
    genesisx.libdir.release = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/release
    genesisx.libdir.profile = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/profile
    QMAKE_LIBDIR += $$genesisx.libdir.debug $$genesisx.libdir.release $$genesisx.libdir.profile

message(LIBDIRS: $$genesisx.libdir.debug $$genesisx.libdir.release $$genesisx.libdir.profile)
    # Android vs Desktop library names
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
        LIBS += -lgenesisx_$${GX_ABI}
    } else:contains(QMAKE_HOST.os, Linux) {
        LIBS += -lgenesisx
    } else {
        LIBS += -lgenesisx
        # msvc:CONFIG(debug,debug|release): LIBS += -lGenesisXd
    }

    # Optional QML import
    GENESISX_QML_IMPORT = $$clean_path($$GENESISX_BUILD_ROOT/core/qml)
    exists($$GENESISX_QML_IMPORT) {
        QML_IMPORT_PATH += $$GENESISX_QML_IMPORT
        DEFINES += GENESISX_QML_DIR=\\\"$$GENESISX_QML_IMPORT\\\"
    }

    contains(QMAKE_HOST.os, Windows) {
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
        GX_DLL_DEST_DEBUG    = $$shell_path($$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/debug/genesisx.dll)
        GX_DLL_DEST_RELEASE  = $$shell_path($$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/release/genesisx.dll)
        GX_DLL_DEST_PROFILE  = $$shell_path($$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/profile/genesisx.dll)

        GX_PHYS_DLL_DEST_DEBUG    = $$shell_path($$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/debug/genesisx_physics.dll)
        GX_PHYS_DLL_DEST_RELEASE  = $$shell_path($$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/release/genesisx_physics.dll)
        GX_PHYS_DLL_DEST_PROFILE  = $$shell_path($$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/profile/genesisx_physics.dll)

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


    # contains(QMAKE_HOST.os, Linux) {
    #     # Where Genesis-X builds its libs
    #     GX_LIB_BASE = $$clean_path($$GENESISX_BUILD_ROOT/bin/linux/$$COMPILER_PATH/$$PROCESSOR_PATH)
    #     GX_LIB_BASE = $$replace(GX_LIB_BASE, \\\\, /)

    #     # Where the app binaries live
    #     GX_APP_BASE = $$clean_path($$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH)
    #     GX_APP_BASE = $$replace(GX_APP_BASE, \\\\, /)

    #     # Ensure runtime can find .so next to the exe
    #     QMAKE_LFLAGS += -Wl,-rpath,\$$ORIGIN

    #     message(for linux activated?)
    #     # for(build, "debug" "release" "profile") {
    #         SRC_DIR = $$GX_LIB_BASE/debug
    #         DST_DIR = $$GX_APP_BASE/debug

    #         # Make sure destination exists (won’t error if it already does)
    #         QMAKE_POST_LINK += $$escape_expand(\\n\\t)mkdir -p "$$DST_DIR"
    #         message(SRC_DIR $$SRC_DIR DST_DIR $$DST_DIR)
    #         # Copy ALL soname variants (*.so, *.so.1, *.so.1.0.0) — ignore if nothing matches
    #         QMAKE_POST_LINK += $$escape_expand(\\n\\t)sh -c 'cp -f "$$SRC_DIR"/libgenesisx.so* "$$DST_DIR"/ 2>/dev/null || true'
    #         QMAKE_POST_LINK += $$escape_expand(\\n\\t)sh -c 'cp -f "$$SRC_DIR"/libgenesisx_physics.so* "$$DST_DIR"/ 2>/dev/null || true'
    #     # }
    # }

    linux:!android {
        # Embed a runpath that points to the exe directory at runtime
        QMAKE_LFLAGS += -Wl,-rpath,\$$ORIGIN
        # (Optional) also add specific config dirs if you want:
        QMAKE_RPATHDIR += $$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/debug
        QMAKE_RPATHDIR += $$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/release
        QMAKE_RPATHDIR += $$APP_OUTPUT_DIR/binaries/$$LOCAL_DESTINATION_PATH/profile
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
        # ABI as reported by Qt (e.g. arm64-v8a)
        # GX_ABI = $$QT_ARCH
        # isEmpty(GX_ABI): GX_ABI = arm64-v8a

        # Candidate libdirs (we add all three; only existing files are taken)
        GX_DIRS = \
            $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/debug \
            $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/release \
            $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/profile

        # Our shared object name on Android
        for(dir, GX_DIRS) {
            GX_SO = $$dir/libgenesisx_$${GX_ABI}.so
            exists($$GX_SO) {
                ANDROID_EXTRA_LIBS += $$GX_SO
                message([genesisx] add extra library ANDROID_EXTRA_LIBS += $$GX_SO)
            }
        }

        # If GenesisX depends on Firebase .so’s, include them too so the APK contains them
        GX_FB_ROOT = $$clean_path($$GENESISX_BUILD_ROOT/3rdparty/firebase_cpp_sdk/libs/android/$$GX_ABI)
        GX_FB_LIST = \
            $$GX_FB_ROOT/libfirebase_app.so \
            $$GX_FB_ROOT/libfirebase_messaging.so \
            $$GX_FB_ROOT/libfirebase_auth.so

        for(fb, GX_FB_LIST) {
            exists($$fb) {
                ANDROID_EXTRA_LIBS += $$fb
                message([genesisx] add extra library ANDROID_EXTRA_LIBS += $$fb)
            }
        }

        # Copy our Gradle include into the app's android/ dir automatically
        GX_ANDROID_GRADLE_SRC = $$clean_path($$GENESISX_BUILD_ROOT/core/android-template)
        GX_ANDROID_GRADLE_DST = $$APP_OUTPUT_DIR/android

        GX_GSERV_SRC =
        exists($$GX_ANDROID_GRADLE_DST/google-services.json) {
            GX_GSERV_SRC = $$clean_path($$GX_ANDROID_GRADLE_DST/google-services.json)
        } else: exists($$GX_ANDROID_GRADLE_SRC/google-services.json) {
            GX_GSERV_SRC = $$clean_path($$GX_ANDROID_GRADLE_SRC/google-services.json)
        }
        GX_GSERV_DST = $$clean_path($$GX_ANDROID_GRADLE_DST/google-services.json)
        GX_GBUILD_SRC = $$GX_ANDROID_GRADLE_SRC/build.gradle
        GX_GBUILD_DST = $$GX_ANDROID_GRADLE_DST/build.gradle
        GX_GPROPERTIES_SRC = $$GX_ANDROID_GRADLE_SRC/gradle.properties
        GX_GPROPERTIES_DST = $$GX_ANDROID_GRADLE_DST/gradle.properties

        GX_ANDROID_GRADLE_SRC = $$shell_path($$GX_ANDROID_GRADLE_SRC)
        GX_ANDROID_GRADLE_DST = $$shell_path($$GX_ANDROID_GRADLE_DST)
        GX_GSERV_SRC = $$shell_path($$GX_GSERV_SRC)
        GX_GSERV_DST = $$shell_path($$GX_GSERV_DST)
        GX_GBUILD_SRC = $$shell_path($$GX_GBUILD_SRC)
        GX_GBUILD_DST = $$shell_path($$GX_GBUILD_DST)
        GX_GPROPERTIES_SRC = $$shell_path($$GX_GPROPERTIES_SRC)
        GX_GPROPERTIES_DST = $$shell_path($$GX_GPROPERTIES_DST)

        # ---------- Run automatically before link ----------
        if (!exists($$GX_ANDROID_GRADLE_DST)) {
            QMAKE_PRE_LINK += \
                $$escape_expand(\\n\\t)$(MKDIR) $$shell_quote($$GX_ANDROID_GRADLE_DST)
        }
        QMAKE_PRE_LINK += \
            $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_GBUILD_SRC) $$shell_quote($$GX_GBUILD_DST) \
            $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_GPROPERTIES_SRC) $$shell_quote($$GX_GPROPERTIES_DST)

        isEmpty(GX_GSERV_DST) {
            QMAKE_PRE_LINK += \
                $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_GSERV_SRC) $$shell_quote($$GX_GSERV_DST)
            message([genesisx] will copy google-services.json from: $$GX_GSERV_SRC)
        } else {
            message([genesisx] WARNING: google-services.json not found in template or existing in app)
        }

        # ---------- Optional manual target (handy for testing) ----------
        gx_prepare_android.target = prepare_android_genesisx
        gx_prepare_android.commands = \
            $(MKDIR) $$GX_ANDROID_GRADLE_DST
        gx_prepare_android.commands += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_GBUILD_SRC) $$shell_quote($$GX_GBUILD_DST)
        gx_prepare_android.commands += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_GPROPERTIES_SRC) $$shell_quote($$GX_GPROPERTIES_DST)

        !isEmpty(GX_GSERV_SRC) {
            gx_prepare_android.commands += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_GSERV_SRC) $$shell_quote($$GX_GSERV_DST)
            message([genesisx] will copy google-services.json from: $$GX_GSERV_SRC)
        } else {
            message([genesisx] WARNING: google-services.json not found in app or template)
        }
        QMAKE_EXTRA_TARGETS += gx_prepare_android
        # first.depends += prepare_android_genesisx
    }

    message([genesisx] libdirs = $$genesisx.libdir.debug | $$genesisx.libdir.release | $$genesisx.libdir.profile | BUILD_PATH = $$BUILD_PATH | abi = $${GX_ABI} and arch = $$QT_ARCH)
}


# !defined(__GX_CORE_PRI_INCLUDED__, var) {
#     __GX_CORE_PRI_INCLUDED__ = 1
#     message([genesisx] module loaded from $$PWD)

#     GENESISX_BUILD_ROOT = $$clean_path($$PWD/../..)
#     include($$GENESISX_BUILD_ROOT/common/qmake-target-platform.pri)
#     include($$GENESISX_BUILD_ROOT/common/qmake-destination-path.pri)

#     QT.genesisx.name          = genesisx
#     QT.genesisx.depends       = core
#     QT.genesisx.friendly_name = GenesisX (Core)

#     # headers (new layout, fallback to legacy)
#     GX_CORE_INC_ROOT = $$clean_path($$GENESISX_BUILD_ROOT/core/include)
#     !exists($$GX_CORE_INC_ROOT/GenesisX) {
#         GX_CORE_INC_ROOT = $$clean_path($$GENESISX_BUILD_ROOT/include)
#     }
#     INCLUDEPATH += $$GX_CORE_INC_ROOT

#     # ----- lib search roots (your build output tree) -----
#     genesisx.libdir.debug   = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/debug
#     genesisx.libdir.release = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/release
#     genesisx.libdir.profile = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/profile
#     QMAKE_LIBDIR += $$genesisx.libdir.debug $$genesisx.libdir.release $$genesisx.libdir.profile

#     # ----- robust Android detection -----
#     GX_ANDROID =

#     # 1) canonical qmake scope
#     android: GX_ANDROID = 1

#     # 2) variables set by Android mkspecs
#     !isEmpty(ANDROID_PLATFORM): GX_ANDROID = 1
#     contains(QMAKE_PLATFORM, android): GX_ANDROID = 1
#     contains(QMAKE_XSPEC, android):   GX_ANDROID = 1
#     contains(QMAKE_SPEC, android):    GX_ANDROID = 1
#     contains(QMAKESPEC, android):     GX_ANDROID = 1

#     # 3) target kit prefix path (very reliable)
#     QT_PREFIX = $$[QT_INSTALL_PREFIX]
#     contains(QT_PREFIX, /android_)|contains(QT_PREFIX, \\android_) { GX_ANDROID = 1 }

#     # 4) last resort: the compiler path
#     isEmpty(GX_ANDROID) {
#         contains(QMAKE_CXX, aarch64-linux-android) { GX_ANDROID = 1 }
#         else: contains(QMAKE_CXX, clang\\+\\+) {
#             # If this is the NDK clang toolchain path, treat as Android
#             contains(QMAKE_CXX, toolchains/.*/llvm)|contains(QMAKE_CXX, android) { GX_ANDROID = 1 }
#         }
#     }

#     # ABI name used by your filenames
#     GX_ABI = $$QT_ARCH
#     isEmpty(GX_ABI): GX_ABI = arm64-v8a
#     message([GX] BUILD_PATH = $$BUILD_PATH)
#     !isEmpty(GX_ANDROID) {
#         message([genesisx] ANDROID build detected (ABI=$$GX_ABI) $$debug or $$release or $$profile)
#         # Link the actual file by full path to avoid -lGenesisX
#         CONFIG(debug, debug|release|profile): \
#             LIBS += $$quote($$genesisx.libdir.debug/libgenesisx_$${GX_ABI}.so)
#         CONFIG(release, debug|release|profile): \
#             LIBS += $$quote($$genesisx.libdir.release/libgenesisx_$${GX_ABI}.so)
#         CONFIG(profile, debug|release|profile): \
#             LIBS += $$quote($$genesisx.libdir.profile/libgenesisx_$${GX_ABI}.so)
#     } else {
#         message([genesisx] DESKTOP build detected)
#         LIBS += -L$$genesisx.libdir.debug -L$$genesisx.libdir.release -L$$genesisx.libdir.profile -lGenesisX
#     }

#     # Only define this if your core really builds as static
#     # DEFINES += GENESISX_STATIC

#     win32: QT.genesisx.depends += widgets

#     message([genesisx] libdirs = $$genesisx.libdir.debug | $$genesisx.libdir.release | $$genesisx.libdir.profile)
# }


# !defined(__GX_CORE_PRI_INCLUDED__, var) {
#     __GX_CORE_PRI_INCLUDED__ = 1
#     message([genesisx] module loaded from $$PWD)

#     GENESISX_BUILD_ROOT = $$clean_path($$PWD/../..)
#     include($$GENESISX_BUILD_ROOT/common/qmake-target-platform.pri)
#     include($$GENESISX_BUILD_ROOT/common/qmake-destination-path.pri)

#     QT.genesisx.name          = genesisx
#     QT.genesisx.depends       = core
#     QT.genesisx.friendly_name = GenesisX (Core)

#     GX_CORE_INC_ROOT = $$clean_path($$GENESISX_BUILD_ROOT/core/include)
#     !exists($$GX_CORE_INC_ROOT/GenesisX) {
#         GX_CORE_INC_ROOT = $$clean_path($$GENESISX_BUILD_ROOT/include)
#     }
#     INCLUDEPATH += $$GX_CORE_INC_ROOT

#     # ---- lib search roots (built by GenesisX) ----
#     genesisx.libdir.debug   = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/debug
#     genesisx.libdir.release = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/release
#     genesisx.libdir.profile = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/profile
#     QMAKE_LIBDIR += $$genesisx.libdir.debug $$genesisx.libdir.release $$genesisx.libdir.profile

#     # ---- robust Android detection (works even when Spec is empty) ----
#     GX_ANDROID_BUILD =
#     contains(QMAKE_XSPEC, android)|contains(QMAKE_SPEC, android)|contains(QMAKESPEC, android) {
#         GX_ANDROID_BUILD = 1
#     } else: contains(QMAKE_CXX, aarch64-linux-android)|contains(QMAKE_CXX, armv7a-linux-androideabi)|contains(QMAKE_CXX, android)|contains(QMAKE_CXX, Android) {
#         GX_ANDROID_BUILD = 1
#     }

# message(qmake_xspec: $$QMAKE_XSPEC and qmake_spec: $$QMAKE_SPEC and qmakespec: $$QMAKESPEC and qmake_cxx: $$QMAKE_CXX)
#     GX_ABI = $$QT_ARCH
#     isEmpty(GX_ABI): GX_ABI = arm64-v8a   # sensible default for your kit

#     !isEmpty(GX_ANDROID_BUILD) {message(genesisx get android build: $$GX_ANDROID_BUILD)
#         message([genesisx] ANDROID branch active; ABI=$$GX_ABI)

#         # On Android, link the actual .so file by full path (no -lGenesisX)
#         CONFIG(debug, debug|release|profile): \
#             LIBS += $$quote($$genesisx.libdir.debug/libgenesisx_$${GX_ABI}.so)
#         CONFIG(release, debug|release|profile): \
#             LIBS += $$quote($$genesisx.libdir.release/libgenesisx_$${GX_ABI}.so)
#         CONFIG(profile, debug|release|profile): \
#             LIBS += $$quote($$genesisx.libdir.profile/libgenesisx_$${GX_ABI}.so)
#     } else {message(genesisx get windows build: $$GX_ANDROID_BUILD)
#         # Desktop: classic -lGenesisX
#         LIBS += -L$$genesisx.libdir.debug -L$$genesisx.libdir.release -L$$genesisx.libdir.profile -lGenesisX
#     }

#     # If (and only if) your core is static, uncomment:
#     # DEFINES += GENESISX_STATIC

#     win32: QT.genesisx.depends += widgets
#     message([genesisx] libdirs = $$genesisx.libdir.debug | $$genesisx.libdir.release | $$genesisx.libdir.profile)
# }



# !defined(__GX_CORE_PRI_INCLUDED__, var) {
#     __GX_CORE_PRI_INCLUDED__ = 1

#     message([genesisx] module loaded from $$PWD)

#     # repo root (this file is at mkspecs/modules)
#     GENESISX_BUILD_ROOT = $$clean_path($$PWD/../..)
#     include($$GENESISX_BUILD_ROOT/common/qmake-target-platform.pri)
#     include($$GENESISX_BUILD_ROOT/common/qmake-destination-path.pri)

#     # --- make qmake recognize: QT += genesisx ---
#     QT.genesisx.name          = genesisx
#     QT.genesisx.depends       = core          # QtCore only
#     QT.genesisx.friendly_name = GenesisX (Core)

#     # headers (new layout, with fallback)
#     GX_CORE_INC_ROOT = $$clean_path($$GENESISX_BUILD_ROOT/core/include)
#     !exists($$GX_CORE_INC_ROOT/GenesisX) {
#         GX_CORE_INC_ROOT = $$clean_path($$GENESISX_BUILD_ROOT/include)
#     }
#     INCLUDEPATH += $$GX_CORE_INC_ROOT

#     # library search paths
#     genesisx.libdir.debug   = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/debug
#     genesisx.libdir.release = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/release
#     genesisx.libdir.profile = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/profile
#     QMAKE_LIBDIR += $$genesisx.libdir.debug $$genesisx.libdir.release $$genesisx.libdir.profile

#     android {message(using this one????)
#         # qmake names Android libs: libgenesisx_<abi>.so
#         GENESISX_ABI = $$QT_ARCH
#         isEmpty(GENESISX_ABI): GENESISX_ABI = arm64-v8a

#         CONFIG(debug, debug|release|profile): \
#             LIBS += $$quote($$genesisx.libdir.debug/libgenesisx_$${GENESISX_ABI}.so)
#         CONFIG(release, debug|release|profile): \
#             LIBS += $$quote($$genesisx.libdir.release/libgenesisx_$${GENESISX_ABI}.so)
#         CONFIG(profile, debug|release|profile): \
#             LIBS += $$quote($$genesisx.libdir.profile/libgenesisx_$${GENESISX_ABI}.so)
#     } else {message(or the win??)
#         LIBS += -L$$genesisx.libdir.debug -L$$genesisx.libdir.release -L$$genesisx.libdir.profile -lGenesisX
#     }
#     # msvc:CONFIG(debug,debug|release): LIBS += -lGenesisXd   # if your debug lib has a 'd' suffix

#     # QML imports (core-only; avoids physics QML)
#     GENESISX_QML_IMPORT = $$clean_path($$GENESISX_BUILD_ROOT/core/qml)
#     !exists($$GENESISX_QML_IMPORT) {
#         # optional legacy fallback; safe if you kept core-only QML here
#         GENESISX_QML_IMPORT = $$clean_path($$GENESISX_BUILD_ROOT/qml)
#     }
#     exists($$GENESISX_QML_IMPORT) {
#         QML_IMPORT_PATH += $$GENESISX_QML_IMPORT
#         DEFINES += GENESISX_QML_DIR=\\\"$$GENESISX_QML_IMPORT\\\"
#     }

#     # static build toggle (only if true for your build)
#     DEFINES += GENESISX_STATIC

#     win32 {
#         QT.genesisx.depends += widgets
#     }

#     message([genesisx] libdirs = $$genesisx.libdir.debug | $$genesisx.libdir.release | $$genesisx.libdir.profile)
# }

