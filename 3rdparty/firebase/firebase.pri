# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

FIREBASE_HOME = $$clean_path($$PWD/../firebase_cpp_sdk)

!isEmpty(FIREBASE_CPP_SDK_DIR): FIREBASE_HOME = $$clean_path($$FIREBASE_CPP_SDK_DIR)
# !isEmpty($$system_path($$getenv(FIREBASE_CPP_SDK_DIR))): FIREBASE_HOME = $$clean_path($$system_path($$getenv(FIREBASE_CPP_SDK_DIR))) // need attension

INCLUDEPATH += $$FIREBASE_HOME/include $$FIREBASE_HOME/libs

android {
    contains(QT_ARCH, arm64-v8a) {
        FIREBASE_ABI_DIR = $$FIREBASE_HOME/libs/android/arm64-v8a
    } else: contains(QT_ARCH, x86_64) {
        FIREBASE_ABI_DIR = $$FIREBASE_HOME/libs/android/x86_64
    } else {
        warning([Firebase] Unknown QT_ARCH: $$ANDROID_ABI $$QT_ARCH)
    }

    LIBS += -L$$FIREBASE_ABI_DIR -lfirebase_messaging -lfirebase_app -lfirebase_auth
    LIBS += -llog -lz -lm -ldl

    !exists($$FIREBASE_HOME/include/firebase/app.h) {
        warning([Firebase] SDK not found at $$FIREBASE_HOME - set $$FIREBASE_CPP_SDK_DIR or place the SDK under 3rdparty/firebase_cpp_sdk)
    }
}

macos|ios {
    firebase_ios_root = $$PWD/../firebase_ios_sdk
    firebase_ios_msg = $$files($$firebase_ios_root/FirebaseMessaging/FirebaseMessaging.xcframework)
    !isEmpty(firebase_ios_msg) {
        DEFINES += GX_HAVE_FIREBASE=1
    } else {
        DEFINES += GX_HAVE_FIREBASE=0
    }

    FIREBASE_IOS_HOME = $$clean_path($$PWD/../firebase_ios_sdk)

    # C++ SDK libs for Apple
    macos {
        FIREBASE_LIB_DIR = $$FIREBASE_HOME/libs/darwin/universal
    }
    ios {
        FIREBASE_LIB_DIR = $$FIREBASE_HOME/libs/ios/universal
    }

    LIBS += -L$$FIREBASE_LIB_DIR -lfirebase_app -lfirebase_messaging -lfirebase_auth

    LIBS += -framework CoreFoundation \
            -framework Security \
            -framework SystemConfiguration \
            -framework CFNetwork

    QMAKE_LFLAGS += -ObjC
}

macos {
    FIREBASECORE_XC = $$FIREBASE_IOS_HOME/FirebaseAnalytics/FirebaseCore.xcframework
    FIREBASECOREEXT_XC  = $$FIREBASE_IOS_HOME/FirebaseAnalytics/FirebaseCoreExtension.xcframework
    FIREBASEMSG_XC  = $$FIREBASE_IOS_HOME/FirebaseMessaging/FirebaseMessaging.xcframework

    FIREBASECORE_FW = $$FIREBASECORE_XC/macos-arm64_x86_64/FirebaseCore.framework
    FIREBASECOREEXT_FW  = $$FIREBASECOREEXT_XC/macos-arm64_x86_64/FirebaseCoreExtension.framework
    FIREBASEMSG_FW  = $$FIREBASEMSG_XC/macos-arm64_x86_64/FirebaseMessaging.framework

    INCLUDEPATH += \
        $$FIREBASECORE_FW/Headers \
        $$FIREBASECOREEXT_FW/Headers \
        $$FIREBASEMSG_FW/Headers

    FIREBASECORE_FW_ROOT = $$dirname(FIREBASECORE_FW)
    FIREBASECOREEXT_FW_ROOT = $$dirname(FIREBASECOREEXT_FW)
    FIREBASEMSG_FW_ROOT  = $$dirname(FIREBASEMSG_FW)

    QMAKE_OBJECTIVE_CFLAGS   += -F$$FIREBASECORE_FW_ROOT -F$$FIREBASECOREEXT_FW_ROOT -F$$FIREBASEMSG_FW_ROOT
    QMAKE_OBJECTIVE_CXXFLAGS += -F$$FIREBASECORE_FW_ROOT -F$$FIREBASECOREEXT_FW_ROOT -F$$FIREBASEMSG_FW_ROOT
    QMAKE_LFLAGS             += -F$$FIREBASECORE_FW_ROOT -F$$FIREBASECOREEXT_FW_ROOT -F$$FIREBASEMSG_FW_ROOT

    LIBS += -framework FirebaseCore \
            -framework FirebaseCoreExtension \
            -framework FirebaseMessaging

    message($$LIBS)
}

ios {
    # DEFINES += GX_HAVE_FIREBASE=0
    FIREBASECORE_XC = $$FIREBASE_IOS_HOME/FirebaseAnalytics/FirebaseCore.xcframework
    FIREBASEMSG_XC  = $$FIREBASE_IOS_HOME/FirebaseMessaging/FirebaseMessaging.xcframework

    FIREBASECORE_FW = $$FIREBASECORE_XC/ios-arm64/FirebaseCore.framework
    FIREBASEMSG_FW  = $$FIREBASEMSG_XC/ios-arm64/FirebaseMessaging.framework

    INCLUDEPATH += \
        $$FIREBASECORE_FW/Headers \
        $$FIREBASEMSG_FW/Headers

    FIREBASECORE_FW_ROOT = $$dirname(FIREBASECORE_FW)
    FIREBASEMSG_FW_ROOT  = $$dirname(FIREBASEMSG_FW)

    QMAKE_OBJECTIVE_CFLAGS   += -F$$FIREBASECORE_FW_ROOT -F$$FIREBASEMSG_FW_ROOT
    QMAKE_OBJECTIVE_CXXFLAGS += -F$$FIREBASECORE_FW_ROOT -F$$FIREBASEMSG_FW_ROOT
    QMAKE_LFLAGS             += -F$$FIREBASECORE_FW_ROOT -F$$FIREBASEMSG_FW_ROOT

    LIBS += -framework FirebaseCore -framework FirebaseMessaging
}

# macos {
#     FIREBASE_IOS_HOME = $$clean_path($$PWD/../firebase_ios_sdk)
#     FIREBASE_LIB_DIR = $$FIREBASE_HOME/libs/darwin/universal

#     INCLUDEPATH += $$FIREBASE_IOS_HOME/FirebaseAnalytics/FirebaseCore.xcframework/ios-arm64/FirebaseCore.framework/Headers

#     contains(QT_ARCH, arm64) {
#         FIREBASE_ABI_DIR = $$FIREBASE_HOME/libs/android/arm64-v8a
#     } else: contains(QT_ARCH, x86_64) {
#         FIREBASE_ABI_DIR = $$FIREBASE_HOME/libs/android/x86_64
#     } else {
#         FIREBASE_ABI_DIR = $$FIREBASE_HOME/libs/android/universal
#     }

#     LIBS += -L$$FIREBASE_ABI_DIR -lfirebase_messaging -lfirebase_app -lfirebase_auth

#     LIBS += -framework CoreFoundation \
#             -framework Security \
#             -framework SystemConfiguration \
#             -framework CFNetwork
# }

# ios {
#     FIREBASE_IOS_HOME = $$clean_path($$PWD/../firebase_ios_sdk)
#     FIREBASE_LIB_DIR = $$FIREBASE_HOME/libs/ios/universal

#     INCLUDEPATH += $$FIREBASE_IOS_HOME/FirebaseAnalytics/FirebaseCore.xcframework/ios-arm64/FirebaseCore.framework/Headers

#     contains(QT_ARCH, arm64) {
#         FIREBASE_ABI_DIR = $$FIREBASE_HOME/libs/android/device-arm64
#     } else {
#         FIREBASE_ABI_DIR = $$FIREBASE_HOME/libs/android/universal
#     }

#     LIBS += -L$$FIREBASE_ABI_DIR -lfirebase_messaging -lfirebase_app -lfirebase_auth

#     LIBS += -framework CoreFoundation \
#             -framework Security \
#             -framework SystemConfiguration \
#             -framework CFNetwork
# }
