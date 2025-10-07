FIREBASE_HOME = $$clean_path($$PWD/../firebase_cpp_sdk)

!isEmpty(FIREBASE_CPP_SDK_DIR): FIREBASE_HOME = $$clean_path($$FIREBASE_CPP_SDK_DIR)
# !isEmpty($$system_path($$getenv(FIREBASE_CPP_SDK_DIR))): FIREBASE_HOME = $$clean_path($$system_path($$getenv(FIREBASE_CPP_SDK_DIR))) // need attension

INCLUDEPATH += $$FIREBASE_HOME/include $$FIREBASE_HOME/libs

android {
    contains(QT_ARCH, arm64-v8a) {
        FIREBASE_ABI_DIR = $$FIREBASE_HOME/libs/android/arm64-v8a
    } else {
        warning([Firebase] Unknown QT_ARCH: $$ANDROID_ABI $$QT_ARCH)
    }

    LIBS += -L$$FIREBASE_ABI_DIR -lfirebase_messaging -lfirebase_app -lfirebase_auth
    LIBS += -llog -lz -lm -ldl

    !exists($$FIREBASE_HOME/include/firebase/app.h) {
        warning([Firebase] SDK not found at $$FIREBASE_HOME - set $$FIREBASE_CPP_SDK_DIR or place the SDK under 3rdparty/firebase_cpp_sdk)
    }
}
