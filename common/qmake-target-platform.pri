# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

# ---------- OS ----------
equals(QMAKE_HOST.os, Windows) {
    PLATFORM_PATH = windows
} else: equals(QMAKE_HOST.os, Darwin) {
    PLATFORM_PATH = osx
} else: equals(QMAKE_HOST.os, Linux) {
    PLATFORM_PATH = linux
} else {
    PLATFORM_PATH = unknown-platform
}

# ---------- Compiler / toolchain ----------
# Try several spec vars; some environments set different ones
SPEC = $$QMAKE_XSPEC
isEmpty(SPEC): SPEC = $$QMAKE_SPEC
isEmpty(SPEC): SPEC = $$QMAKESPEC

COMPILER_PATH = unknown-compiler
contains(SPEC, android) { COMPILER_PATH = android-clang }
else: contains(SPEC, clang) { COMPILER_PATH = clang }
else: contains(SPEC, g++)  { COMPILER_PATH = gcc }
else: contains(SPEC, msvc) { COMPILER_PATH = msvc }

# Fallback based on the actual compiler command
isEqual(COMPILER_PATH, unknown-compiler) {
    contains(QMAKE_CXX, clang) { COMPILER_PATH = clang }
    else: contains(QMAKE_CXX, clang++) {
        contains(PLATFORM_PATH, windows) {
            COMPILER_PATH = clangcc
        } else {
            COMPILER_PATH = android-clangcc
        }
    }
    else: contains(QMAKE_CXX, g++) { COMPILER_PATH = gcc }
    else: contains(QMAKE_CXX, cl) { COMPILER_PATH = msvc }
    # else: COMPILER_PATH = android-clangcc
}

# alternative setting compiler
isEqual(COMPILER_PATH, unknown-compiler) {
    contains(PLATFORM_PATH, windows) {
        COMPILER_PATH = clangcc
    } else:contains(SPEC, android) {
        message(on android)
    } else {
        message($$COMPILER_PATH and $$QMAKE_CXX still to do)
    }
}

# ---------- Architecture ----------
ARCH = $$QT_ARCH
isEmpty(ARCH): ARCH = $$QMAKE_TARGET.arch
isEmpty(ARCH): ARCH = $$QMAKE_HOST.arch

PROCESSOR_PATH = unknown-processor
contains(ARCH, arm64)|contains(ARCH, aarch64)|equals(ARCH, arm64)|equals(ARCH, arm64-v8a) {
    PROCESSOR_PATH = arm64-v8a
    ARCH = arm64-v8a
}
else: contains(ARCH, 64)|equals(ARCH, x86_64) { PROCESSOR_PATH = x64 }
else: contains(ARCH, 86)|equals(ARCH, i386)|equals(ARCH, i686) { PROCESSOR_PATH = x86 }

# ---------- Build type ----------
CONFIG(debug, debug|release|profile) {
    BUILD_PATH = debug
} else: CONFIG(profile, debug|release|profile) {
    BUILD_PATH = profile
} else {
    BUILD_PATH = release
}
# QMAKEFEATURES += ../mkspecs/features/gx_app_root.prf

APP_OUTPUT_DIR = $$ANDROID_PACKAGE_SOURCE_DIR

# Debug echo
message(Platform: $$PLATFORM_PATH  |  Spec: $$SPEC  |  CXX: $$QMAKE_CXX  |  ProcessorPath: $$PROCESSOR_PATH  |  Arch: $$ARCH)

