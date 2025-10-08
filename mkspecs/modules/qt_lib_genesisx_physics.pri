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


# !defined(__GX_PHYS_PRI_INCLUDED__, var) {
#     __GX_PHYS_PRI_INCLUDED__ = 1
#     message([genesisx_physics] module loaded from $$PWD)

#     GENESISX_BUILD_ROOT = $$clean_path($$PWD/../..)
#     include($$GENESISX_BUILD_ROOT/common/qmake-target-platform.pri)
#     include($$GENESISX_BUILD_ROOT/common/qmake-destination-path.pri)

#     QT.genesisx_physics.name          = genesisx_physics
#     QT.genesisx_physics.depends       = core gui
#     QT.genesisx_physics.friendly_name = GenesisX (Physics)

#     INCLUDEPATH += $$clean_path($$GENESISX_BUILD_ROOT/physics/include)

#     phys.libdir.debug   = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/debug
#     phys.libdir.release = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/release
#     phys.libdir.profile = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/profile
#     QMAKE_LIBDIR += $$phys.libdir.debug $$phys.libdir.release $$phys.libdir.profile

#     # Detect Android exactly like core
#     GX_ANDROID =
#     android: GX_ANDROID = 1
#     !isEmpty(ANDROID_PLATFORM): GX_ANDROID = 1
#     contains(QMAKE_PLATFORM, android): GX_ANDROID = 1
#     contains(QMAKE_XSPEC, android):   GX_ANDROID = 1
#     contains(QMAKE_SPEC, android):    GX_ANDROID = 1
#     contains(QMAKESPEC, android):     GX_ANDROID = 1
#     QT_PREFIX = $$[QT_INSTALL_PREFIX]
#     contains(QT_PREFIX, /android_)|contains(QT_PREFIX, \\android_) { GX_ANDROID = 1 }
#     isEmpty(GX_ANDROID) {
#         contains(QMAKE_CXX, aarch64-linux-android) { GX_ANDROID = 1 }
#         else: contains(QMAKE_CXX, clang\\+\\+) {
#             contains(QMAKE_CXX, toolchains/.*/llvm)|contains(QMAKE_CXX, android) { GX_ANDROID = 1 }
#         }
#     }

#     GX_ABI = $$QT_ARCH
#     isEmpty(GX_ABI): GX_ABI = arm64-v8a

#     !isEmpty(GX_ANDROID) {
#         message([genesisx_physics] ANDROID build detected (ABI=$$GX_ABI))

#         CONFIG(debug, debug|release|profile): \
#             LIBS += $$quote($$phys.libdir.debug/libgenesisx_physics_$${GX_ABI}.so)
#         CONFIG(release, debug|release|profile): \
#             LIBS += $$quote($$phys.libdir.release/libgenesisx_physics_$${GX_ABI}.so)
#         CONFIG(profile, debug|release|profile): \
#             LIBS += $$quote($$phys.libdir.profile/libgenesisx_physics_$${GX_ABI}.so)

#         # physics depends on core too
#         genesisx.libdir.debug   = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/debug
#         genesisx.libdir.release = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/release
#         genesisx.libdir.profile = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/profile

#         CONFIG(debug, debug|release|profile): \
#             LIBS += $$quote($$genesisx.libdir.debug/libgenesisx_$${GX_ABI}.so)
#         CONFIG(release, debug|release|profile): \
#             LIBS += $$quote($$genesisx.libdir.release/libgenesisx_$${GX_ABI}.so)
#         CONFIG(profile, debug|release|profile): \
#             LIBS += $$quote($$genesisx.libdir.profile/libgenesisx_$${GX_ABI}.so)
#     } else {
#         message([genesisx_physics] DESKTOP build detected)
#         LIBS += -L$$phys.libdir.debug -L$$phys.libdir.release -L$$phys.libdir.profile -lGenesisXPhysics
#         LIBS += -L$$genesisx.libdir.debug -L$$genesisx.libdir.release -L$$genesisx.libdir.profile -lGenesisX
#     }
# }


# !defined(__GX_PHYS_PRI_INCLUDED__, var) {
#     __GX_PHYS_PRI_INCLUDED__ = 1
#     message([genesisx_physics] module loaded from $$PWD)

#     GENESISX_BUILD_ROOT = $$clean_path($$PWD/../..)
#     include($$GENESISX_BUILD_ROOT/common/qmake-target-platform.pri)
#     include($$GENESISX_BUILD_ROOT/common/qmake-destination-path.pri)

#     QT.genesisx_physics.name          = genesisx_physics
#     QT.genesisx_physics.depends       = core gui
#     QT.genesisx_physics.friendly_name = GenesisX (Physics)

#     INCLUDEPATH += $$clean_path($$GENESISX_BUILD_ROOT/physics/include)

#     phys.libdir.debug   = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/debug
#     phys.libdir.release = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/release
#     phys.libdir.profile = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/profile
#     QMAKE_LIBDIR += $$phys.libdir.debug $$phys.libdir.release $$phys.libdir.profile

#     # Detect Android like above
#     GX_ANDROID_BUILD =
#     contains(QMAKE_XSPEC, android)|contains(QMAKE_SPEC, android)|contains(QMAKESPEC, android) {
#         GX_ANDROID_BUILD = 1
#     } else: contains(QMAKE_CXX, aarch64-linux-android)|contains(QMAKE_CXX, armv7a-linux-androideabi)|contains(QMAKE_CXX, android) {
#         GX_ANDROID_BUILD = 1
#     }

#     GX_ABI = $$QT_ARCH
#     isEmpty(GX_ABI): GX_ABI = arm64-v8a

#     !isEmpty(GX_ANDROID_BUILD) {message(genesisx_physics get android build: $$GX_ANDROID_BUILD)
#         message([genesisx_physics] ANDROID branch active; ABI=$$GX_ABI)

#         # physics itself:
#         CONFIG(debug, debug|release|profile): \
#             LIBS += $$quote($$phys.libdir.debug/libgenesisx_physics_$${GX_ABI}.so)
#         CONFIG(release, debug|release|profile): \
#             LIBS += $$quote($$phys.libdir.release/libgenesisx_physics_$${GX_ABI}.so)
#         CONFIG(profile, debug|release|profile): \
#             LIBS += $$quote($$phys.libdir.profile/libgenesisx_physics_$${GX_ABI}.so)

#         # and depend on core too:
#         genesisx.libdir.debug   = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/debug
#         genesisx.libdir.release = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/release
#         genesisx.libdir.profile = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/profile

#         CONFIG(debug, debug|release|profile): \
#             LIBS += $$quote($$genesisx.libdir.debug/libgenesisx_$${GX_ABI}.so)
#         CONFIG(release, debug|release|profile): \
#             LIBS += $$quote($$genesisx.libdir.release/libgenesisx_$${GX_ABI}.so)
#         CONFIG(profile, debug|release|profile): \
#             LIBS += $$quote($$genesisx.libdir.profile/libgenesisx_$${GX_ABI}.so)
#     } else {message(genesisx_physics get windows build: $$GX_ANDROID_BUILD)
#         # Desktop
#         LIBS += -L$$phys.libdir.debug -L$$phys.libdir.release -L$$phys.libdir.profile -lGenesisXPhysics
#         LIBS += -L$$genesisx.libdir.debug -L$$genesisx.libdir.release -L$$genesisx.libdir.profile -lGenesisX
#     }
# }


# !defined(__GX_PHYSICS_PRI_INCLUDED__, var) {
#     __GX_PHYSICS_PRI_INCLUDED__ = 1

#     message([genesisx_physics] module loaded from $$PWD)

#     # Same layout assumption as core
#     GENESISX_BUILD_ROOT = $$clean_path($$PWD/../..)
#     include($$GENESISX_BUILD_ROOT/common/qmake-target-platform.pri)
#     include($$GENESISX_BUILD_ROOT/common/qmake-destination-path.pri)

#     # --- Metadata so `QT += genesisx_physics` works ---
#     QT.genesisx_physics.name           = genesisx_physics
#     QT.genesisx_physics.depends        = genesisx        # pulls core automatically
#     QT.genesisx_physics.friendly_name  = GenesisX Physics

#     # Headers (add a Physics subdir if you keep them separate)
#     GX_CORE_INC_ROOT    = $$clean_path($$GENESISX_BUILD_ROOT/core/include)
#     !exists($$GX_CORE_INC_ROOT) {
#         GX_CORE_INC_ROOT = $$clean_path($$GENESISX_BUILD_ROOT/include)
#     }

#     GX_PHYSICS_INC_ROOT = $$clean_path($$GENESISX_BUILD_ROOT/physics/include)
#     # if you keep physics headers right under physics/include (without /GenesisX), use that instead:
#     #!exists($$GX_PHYSICS_INC_DIR): GX_PHYSICS_INC_DIR = $$clean_path($$GENESISX_BUILD_ROOT/physics/include)

#     INCLUDEPATH += $$GX_CORE_INC_ROOT $$GX_PHYSICS_INC_ROOT

#     # Search paths mirroring core
#     genesisx_physics.libdir.debug   = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/debug
#     genesisx_physics.libdir.release = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/release
#     genesisx_physics.libdir.profile = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/profile
#     QMAKE_LIBDIR += $$genesisx_physics.libdir.debug $$genesisx_physics.libdir.release $$genesisx_physics.libdir.profile

#     # ---- Choose how you deliver Physics ----
#     # A) As a linkable library:
#     #    set the lib name here (override from your .pro if different)
#     # GENESISX_PHYSICS_LIB_NAME = GenesisXPhysics
#     # LIBS += -L$$genesisx_physics.libdir.debug -L$$genesisx_physics.libdir.release -L$$genesisx_physics.libdir.profile \
#             # -l$${GENESISX_PHYSICS_LIB_NAME}
#     android {
#         GENESISX_ABI = $$QT_ARCH
#         isEmpty(GENESISX_ABI): GENESISX_ABI = arm64-v8a

#         # physics itself
#         CONFIG(debug, debug|release|profile): \
#             LIBS += $$quote($$phys.libdir.debug/libgenesisx_physics_$${GENESISX_ABI}.so)
#         CONFIG(release, debug|release|profile): \
#             LIBS += $$quote($$phys.libdir.release/libgenesisx_physics_$${GENESISX_ABI}.so)
#         CONFIG(profile, debug|release|profile): \
#             LIBS += $$quote($$phys.libdir.profile/libgenesisx_physics_$${GENESISX_ABI}.so)

#         # and it depends on core too:
#         genesisx.libdir.debug   = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/debug
#         genesisx.libdir.release = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/release
#         genesisx.libdir.profile = $$GENESISX_BUILD_ROOT/bin/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/profile

#         CONFIG(debug, debug|release|profile): \
#             LIBS += $$quote($$genesisx.libdir.debug/libgenesisx_$${GENESISX_ABI}.so)
#         CONFIG(release, debug|release|profile): \
#             LIBS += $$quote($$genesisx.libdir.release/libgenesisx_$${GENESISX_ABI}.so)
#         CONFIG(profile, debug|release|profile): \
#             LIBS += $$quote($$genesisx.libdir.profile/libgenesisx_$${GENESISX_ABI}.so)
#     } else {
#         LIBS += -L$$phys.libdir.debug -L$$phys.libdir.release -L$$phys.libdir.profile -lGenesisXPhysics
#          # also depend on core
#         LIBS += -L$$genesisx.libdir.debug -L$$genesisx.libdir.release -L$$genesisx.libdir.profile -lGenesisX
#     }
#     # msvc:CONFIG(debug,debug|release): LIBS += -l$${GENESISX_PHYSICS_LIB_NAME}d  # if you use *d suffix

#     # B) (Optional) If you ship Physics only as a plugin you don’t link to:
#     #    Comment out LIBS above and make sure your runtime can find the plugin.
#     #    You can still expose a QML import path if relevant:
#     GENESISX_PHYSICS_QML_IMPORT = $$GENESISX_BUILD_ROOT/physics/qml/GenesisX/Physics
#     QML_IMPORT_PATH += $$GENESISX_PHYSICS_QML_IMPORT
#     DEFINES += GENESISX_PHYSICS_QML_DIR=\\\"$$GENESISX_PHYSICS_QML_IMPORT\\\"

#     # Tag so client code can `#ifdef`
#     DEFINES += GENESISX_HAS_PHYSICS
#     # If building static:
#     # DEFINES += GENESISX_PHYSICS_STATIC

#     message([genesisx_physics] libdirs = $$genesisx_physics.libdir.debug | $$genesisx_physics.libdir.release | $$genesisx_physics.libdir.profile)
# }

