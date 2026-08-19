# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

QT += network sql concurrent

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
win32 {
    OPENSSL_ROOT = D:/Qt/Tools/OpenSSLv3/Win_x64

      INCLUDEPATH += $$OPENSSL_ROOT/include

      # LIBS += \
          # -L$$OPENSSL_ROOT/lib \
          # -lcrypto
    LIBS += $$quote($$OPENSSL_ROOT/lib/libcrypto.lib)
}

# ORM links to core; search same central dir
QMAKE_LIBDIR += $$DESTDIR
# android {
#     ANDROID_OPENSSL_ROOT = D:/Android/Sdk/android_openssl

#     INCLUDEPATH += $$ANDROID_OPENSSL_ROOT/ssl_3/include

#     contains(QT_ARCH, arm64-v8a) {
#         LIBS += -lgenesisx_arm64-v8a
#         # LIBS += -L$$ANDROID_OPENSSL_ROOT/ssl_3/arm64-v8a -lcrypto
#         # LIBS += -L$$ANDROID_OPENSSL_ROOT/ssl_3/arm64-v8a -lcrypto_3
#         OPENSSL_CRYPTO = \
#                 $$ANDROID_OPENSSL_ROOT/ssl_3/arm64-v8a/libcrypto_3.so
#     } else: contains(QT_ARCH, x86_64) {
#         LIBS += -lgenesisx_x86_64
#         # LIBS += -L$$ANDROID_OPENSSL_ROOT/ssl_3/x86_64 -lcrypto
#         # LIBS += -L$$ANDROID_OPENSSL_ROOT/ssl_3/x86_64 -lcrypto_3
#         OPENSSL_CRYPTO = \
#                 $$ANDROID_OPENSSL_ROOT/ssl_3/x86_64/libcrypto_3.so
#     }
#     !exists($$OPENSSL_CRYPTO) {
#         message([genesisx_orm] OpenSSL crypto library not found: $$OPENSSL_CRYPTO)
#     }
#     LIBS += $$OPENSSL_CRYPTO

#     # exists($$ANDROID_OPENSSL_ROOT/openssl.pri) {
#     #     include($$ANDROID_OPENSSL_ROOT/openssl.pri)
#     #     message([genesisx_orm] Android OpenSSL: $$ANDROID_OPENSSL_ROOT)
#     # } else {
#     #     error([genesisx_orm] Android OpenSSL not found at $$ANDROID_OPENSSL_ROOT)
#     # }
android {
    ANDROID_OPENSSL_ROOT = D:/Android/Sdk/android_openssl

    OPENSSL_INCLUDE_DIR = \
        $$ANDROID_OPENSSL_ROOT/ssl_3/include

    !exists($$OPENSSL_INCLUDE_DIR/openssl/evp.h) {
        error([genesisx_orm] OpenSSL headers not found: $$OPENSSL_INCLUDE_DIR)
    }

    INCLUDEPATH += $$OPENSSL_INCLUDE_DIR

    contains(QT_ARCH, arm64-v8a) {
        LIBS += -lgenesisx_arm64-v8a

        OPENSSL_CRYPTO = \
            $$ANDROID_OPENSSL_ROOT/ssl_3/arm64-v8a/libcrypto_3.so
    } else: contains(QT_ARCH, x86_64) {
        LIBS += -lgenesisx_x86_64

        OPENSSL_CRYPTO = \
            $$ANDROID_OPENSSL_ROOT/ssl_3/x86_64/libcrypto_3.so
    } else {
        error([genesisx_orm] Unsupported Android architecture: $$QT_ARCH)
    }

    !exists($$OPENSSL_CRYPTO) {
        error([genesisx_orm] OpenSSL crypto library not found: $$OPENSSL_CRYPTO)
    }

    LIBS += $$quote($$OPENSSL_CRYPTO)

    message([genesisx_orm] OpenSSL crypto library: $$OPENSSL_CRYPTO)
} else {
    LIBS += -lgenesisx
}

linux:!android {
    LIBS += -lcrypto
}

INCLUDEPATH += $$GENESISX_BUILD_ROOT/orm/include

HEADERS += $$files($$PWD/include/GenesisX/*.h, true)

SOURCES += $$files($$PWD/src/*.cpp, true)
