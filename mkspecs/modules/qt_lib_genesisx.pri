# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

# ---- GenesisX (core) qmake module ----
QT.genesisx.name          = genesisx
QT.genesisx.depends       = core
QT.genesisx.friendly_name = GenesisX (Core)

GENESISX_BUILD_ROOT = $$clean_path($$PWD/../..)
include($$GENESISX_BUILD_ROOT/common/qmake-target-platform.pri)
include($$GENESISX_BUILD_ROOT/common/qmake-destination-path.pri)
DEFINES += GX_LOADED_FEATURES
# GX_LOADED_FEATURES = $$QT
# QML_IMPORT_PATH += $$GENESISX_BUILD_ROOT/core/qml
#     GX_CORE_INC_ROOT = $$clean_path($$GENESISX_BUILD_ROOT/core/include)
#     !exists($$GX_CORE_INC_ROOT/GenesisX) { GX_CORE_INC_ROOT = $$clean_path($$GENESISX_BUILD_ROOT/include) }
#     INCLUDEPATH += $$GX_CORE_INC_ROOT

#     # Add all config libdirs in priority order (debug first)
#     genesisx.libdir.debug   = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/debug
#     genesisx.libdir.release = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/release
#     genesisx.libdir.profile = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/profile
#     QMAKE_LIBDIR += $$genesisx.libdir.debug $$genesisx.libdir.release $$genesisx.libdir.profile

# # message(LIBDIRS: $$genesisx.libdir.debug $$genesisx.libdir.release $$genesisx.libdir.profile and $$QMAKE_LIBDIR)
#     # Android vs Desktop library names
#     GX_ANDROID =
#     android: GX_ANDROID = 1
#     !isEmpty(ANDROID_PLATFORM): GX_ANDROID = 1
#     contains(QMAKE_XSPEC, android): GX_ANDROID = 1
#     contains(QMAKE_SPEC, android):  GX_ANDROID = 1
#     contains(QMAKESPEC, android):   GX_ANDROID = 1
#     contains($$[QT_INSTALL_PREFIX], /android_)|contains($$[QT_INSTALL_PREFIX], \\android_) { GX_ANDROID = 1 }

#     GX_ABI = $$QT_ARCH
#     isEmpty(GX_ABI): GX_ABI = arm64-v8a
#     contains(GX_ABI, arm64): GX_ABI = arm64-v8a

#     !isEmpty(GX_ANDROID) {
#         LIBS += -lgenesisx_$${GX_ABI}
#     } else:contains(QMAKE_HOST.os, Linux) {
#         LIBS += -lgenesisx
#     } else {
#         LIBS += -lgenesisx
#         # msvc:CONFIG(debug,debug|release): LIBS += -lGenesisXd
#     }

#     # Optional QML import
#     GENESISX_QML_IMPORT = $$clean_path($$GENESISX_BUILD_ROOT/core/qml)
#     exists($$GENESISX_QML_IMPORT) {
#         QML_IMPORT_PATH += $$GENESISX_QML_IMPORT
#         DEFINES += GENESISX_QML_DIR=\\\"$$GENESISX_QML_IMPORT\\\"
#     }

#     contains(QMAKE_HOST.os, Windows) {
#         # Base directory where DLLs are built
#         GX_LIB_BASE = $$GENESISX_BUILD_ROOT/bin/windows/$$COMPILER_PATH/$$PROCESSOR_PATH

#         # Target DLLs for all build types
#         GX_DLL_DEBUG    = $$shell_path($$GX_LIB_BASE/debug/genesisx.dll)
#         GX_DLL_RELEASE  = $$shell_path($$GX_LIB_BASE/release/genesisx.dll)
#         GX_DLL_PROFILE  = $$shell_path($$GX_LIB_BASE/profile/genesisx.dll)

#         # To DLLs for all build types
#         GX_DLL_DEST_DEBUG    = $$shell_path($$APP_OUTPUT_DIR/../bin/$$LOCAL_DESTINATION_PATH/debug/genesisx.dll)
#         GX_DLL_DEST_RELEASE  = $$shell_path($$APP_OUTPUT_DIR/../bin/$$LOCAL_DESTINATION_PATH/release/genesisx.dll)
#         GX_DLL_DEST_PROFILE  = $$shell_path($$APP_OUTPUT_DIR/../bin/$$LOCAL_DESTINATION_PATH/profile/genesisx.dll)

#         # Post-link copy (only copy those that exist)
#         exists($$GX_DLL_DEBUG): QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_DLL_DEBUG) $$shell_quote($$GX_DLL_DEST_DEBUG)
#         exists($$GX_DLL_RELEASE): QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_DLL_RELEASE) $$shell_quote($$GX_DLL_DEST_RELEASE)
#         exists($$GX_DLL_PROFILE): QMAKE_POST_LINK += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_DLL_PROFILE) $$shell_quote($$GX_DLL_DEST_PROFILE)
#     }

#     contains(QMAKE_HOST.os, Linux) {
#         # Where Genesis-X builds its libs
#         GX_LIB_BASE = $$clean_path($$GENESISX_BUILD_ROOT/bin/linux/$$COMPILER_PATH/$$PROCESSOR_PATH)
#         GX_LIB_BASE = $$replace(GX_LIB_BASE, \\\\, /)

#         # Where your app binaries live
#         GX_APP_BASE = $$clean_path($$APP_OUTPUT_DIR/bin/$$LOCAL_DESTINATION_PATH)
#         GX_APP_BASE = $$replace(GX_APP_BASE, \\\\, /)

#         # Make the runtime loader search the exe directory
#         QMAKE_LFLAGS += -Wl,-rpath,\$$ORIGIN

#         builds = debug
#         builds += release
#         builds += profile
#         for (build, builds) {
#             SRC_DIR = $$GX_LIB_BASE/$$build
#             DST_DIR = $$GX_APP_BASE/$$build
#             # Expand the actual files *now* (qmake time). This returns only existing files.
#             SO_LIST = $$files($$SRC_DIR/libgenesisx.so*)

#             message([genesisx] Linux copy from $$SRC_DIR -> $$DST_DIR)
#             message([genesisx] Linux libs: $$SO_LIST)

#             # Ensure destination exists
#             QMAKE_POST_LINK += $$escape_expand(\\n\\t)mkdir -p $$shell_quote($$DST_DIR)

#             # Emit one cp command per file (no shell loops; robust in Makefiles)
#             for(f, SO_LIST) {
#                 QMAKE_POST_LINK += $$escape_expand(\\n\\t)cp -f $$shell_quote($$f) $$shell_quote($$DST_DIR)/
#             }
#         }
#     }

#     contains(QMAKE_HOST.os, Darwin) {
#         QMAKE_MAC_XCODE_SETTINGS += ALWAYS_SEARCH_USER_PATHS=NO
#         QMAKE_MAC_XCODE_SETTINGS += USE_HEADERMAP=YES
#         QMAKE_MAC_XCODE_SETTINGS += SEPARATE_HEADERMAP=YES

#         LIBS += -framework UserNotifications
#         macos: LIBS += -framework AppKit
#         ios: LIBS += -framework UIKit
#         macos: LIBS += -framework Cocoa
#         LIBS += -framework Foundation
#         QMAKE_LFLAGS += -ObjC
#         QMAKE_OBJECTIVE_CFLAGS += -fobjc-arc
#         QMAKE_OBJECTIVE_CXXFLAGS += -fobjc-arc
#         macos: QMAKE_LFLAGS += -Wl,-undefined,dynamic_lookup

#         # Where Genesis-X builds its libs
#         GX_LIB_BASE = $$clean_path($$GENESISX_BUILD_ROOT/bin/osx/$$COMPILER_PATH/$$PROCESSOR_PATH)
#         GX_LIB_BASE = $$replace(GX_LIB_BASE, \\\\, /)

#         # Where your app binaries live
#         GX_APP_BASE = $$clean_path($$APP_OUTPUT_DIR/bin/$$LOCAL_DESTINATION_PATH)
#         GX_APP_BASE = $$replace(GX_APP_BASE, \\\\, /)

#         # Make the runtime loader search the exe directory
#         # QMAKE_LFLAGS += -Wl,-rpath,\$$ORIGIN
#         # QMAKE_LFLAGS += -rpath

#         builds = debug
#         builds += release
#         builds += profile
#         for (build, builds) {
#             SRC_DIR = $$GX_LIB_BASE/$$build
#             DST_DIR = $$GX_APP_BASE/$$build
#             # Expand the actual files *now* (qmake time). This returns only existing files.
#             SO_LIST = $$files($$SRC_DIR/libgenesisx*)

#             # message([genesisx] Linux copy from $$SRC_DIR -> $$DST_DIR)
#             # message([genesisx] Linux libs: $$SO_LIST)

#             # Ensure destination exists
#             QMAKE_POST_LINK += $$escape_expand(\\n\\t)mkdir -p $$shell_quote($$DST_DIR)

#             # Emit one cp command per file (no shell loops; robust in Makefiles)
#             for(f, SO_LIST) {
#                 # exists($$f) {
#                     QMAKE_POST_LINK += $$escape_expand(\\n\\t)cp -f $$shell_quote($$f) $$shell_quote($$DST_DIR)/
#                 # }
#             }
#         }
#     }

#     linux:!android {
#         # Embed a runpath that points to the exe directory at runtime
#         QMAKE_LFLAGS += -Wl,-rpath,\$$ORIGIN
#         # (Optional) also add specific config dirs if you want:
#         QMAKE_RPATHDIR += $$APP_OUTPUT_DIR/bin/$$LOCAL_DESTINATION_PATH/debug
#         QMAKE_RPATHDIR += $$APP_OUTPUT_DIR/bin/$$LOCAL_DESTINATION_PATH/release
#         QMAKE_RPATHDIR += $$APP_OUTPUT_DIR/bin/$$LOCAL_DESTINATION_PATH/profile
#     }

#     # ---- ANDROID copies ----
#     !isEmpty(GX_ANDROID) {
#         # ABI as reported by Qt (e.g. arm64-v8a)
#         # GX_ABI = $$QT_ARCH
#         # isEmpty(GX_ABI): GX_ABI = arm64-v8a

#         # Candidate libdirs (we add all three; only existing files are taken)
#         GX_DIRS = \
#             $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/debug \
#             $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/release \
#             $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/profile

#         # Our shared object name on Android
#         for(dir, GX_DIRS) {
#             GX_SO = $$dir/libgenesisx_$${GX_ABI}.so
#             exists($$GX_SO) {
#                 ANDROID_EXTRA_LIBS += $$GX_SO
#                 message([genesisx] add extra library ANDROID_EXTRA_LIBS += $$GX_SO)
#             }
#         }

#         # If GenesisX depends on Firebase .so’s, include them too so the APK contains them
#         GX_FB_ROOT = $$clean_path($$GENESISX_BUILD_ROOT/3rdparty/firebase_cpp_sdk/libs/android/$$GX_ABI)
#         GX_FB_LIST = \
#             $$GX_FB_ROOT/libfirebase_app.so \
#             $$GX_FB_ROOT/libfirebase_messaging.so \
#             $$GX_FB_ROOT/libfirebase_auth.so

#         for(fb, GX_FB_LIST) {
#             exists($$fb) {
#                 ANDROID_EXTRA_LIBS += $$fb
#                 message([genesisx] add extra library ANDROID_EXTRA_LIBS += $$fb)
#             }
#         }

#         # Copy our Gradle include into the app's android/ dir automatically
#         GX_ANDROID_GRADLE_SRC = $$clean_path($$GENESISX_BUILD_ROOT/core/android-template)
#         GX_ANDROID_GRADLE_DST = $$APP_OUTPUT_DIR/android

#         GX_GSERV_SRC =
#         exists($$GX_ANDROID_GRADLE_DST/google-services.json) {
#             GX_GSERV_SRC = $$clean_path($$GX_ANDROID_GRADLE_DST/google-services.json)
#         } else: exists($$GX_ANDROID_GRADLE_SRC/google-services.json) {
#             GX_GSERV_SRC = $$clean_path($$GX_ANDROID_GRADLE_SRC/google-services.json)
#         }
#         GX_GSERV_DST = $$clean_path($$GX_ANDROID_GRADLE_DST/google-services.json)
#         GX_GBUILD_SRC = $$GX_ANDROID_GRADLE_SRC/build.gradle
#         GX_GBUILD_DST = $$GX_ANDROID_GRADLE_DST/build.gradle
#         GX_GPROPERTIES_SRC = $$GX_ANDROID_GRADLE_SRC/gradle.properties
#         GX_GPROPERTIES_DST = $$GX_ANDROID_GRADLE_DST/gradle.properties

#         GX_ANDROID_GRADLE_SRC = $$shell_path($$GX_ANDROID_GRADLE_SRC)
#         GX_ANDROID_GRADLE_DST = $$shell_path($$GX_ANDROID_GRADLE_DST)
#         GX_GSERV_SRC = $$shell_path($$GX_GSERV_SRC)
#         GX_GSERV_DST = $$shell_path($$GX_GSERV_DST)
#         GX_GBUILD_SRC = $$shell_path($$GX_GBUILD_SRC)
#         GX_GBUILD_DST = $$shell_path($$GX_GBUILD_DST)
#         GX_GPROPERTIES_SRC = $$shell_path($$GX_GPROPERTIES_SRC)
#         GX_GPROPERTIES_DST = $$shell_path($$GX_GPROPERTIES_DST)

#         # ---------- Run automatically before link ----------
#         if (!exists($$GX_ANDROID_GRADLE_DST)) {
#             QMAKE_PRE_LINK += \
#                 $$escape_expand(\\n\\t)$(MKDIR) $$shell_quote($$GX_ANDROID_GRADLE_DST)
#         }
#         QMAKE_PRE_LINK += \
#             $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_GBUILD_SRC) $$shell_quote($$GX_GBUILD_DST) \
#             $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_GPROPERTIES_SRC) $$shell_quote($$GX_GPROPERTIES_DST)

#         if (!exists($$GX_GSERV_DST)) {
#             QMAKE_PRE_LINK += \
#                 $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_GSERV_SRC) $$shell_quote($$GX_GSERV_DST)
#             message([genesisx!!!!] will copy google-services.json from: $$GX_GSERV_SRC to: $$GX_GSERV_DST)
#         } else {
#             message([genesisx] WARNING: google-services.json not found in template or existing in app to: $$GX_GSERV_DST)
#         }

#         # ---------- Optional manual target (handy for testing) ----------
#         gx_prepare_android.target = prepare_android_genesisx
#         gx_prepare_android.commands = \
#             $(MKDIR) $$GX_ANDROID_GRADLE_DST
#         gx_prepare_android.commands += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_GBUILD_SRC) $$shell_quote($$GX_GBUILD_DST)
#         gx_prepare_android.commands += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_GPROPERTIES_SRC) $$shell_quote($$GX_GPROPERTIES_DST)

#         # !isEmpty(GX_GSERV_SRC) {
#         #     gx_prepare_android.commands += $$escape_expand(\\n\\t)$(COPY_FILE) $$shell_quote($$GX_GSERV_SRC) $$shell_quote($$GX_GSERV_DST)
#         #     message([genesisx] will copy google-services.json from: $$GX_GSERV_SRC)
#         # } else {
#         #     message([genesisx] WARNING: google-services.json not found in app or template)
#         # }
#         QMAKE_EXTRA_TARGETS += gx_prepare_android
#         # first.depends += prepare_android_genesisx
#     }

#     message([genesisx] libdirs = $$genesisx.libdir.debug | $$genesisx.libdir.release | $$genesisx.libdir.profile | BUILD_PATH = $$BUILD_PATH | abi = $${GX_ABI} and arch = $$QT_ARCH)
# }
