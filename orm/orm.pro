QT += network
TEMPLATE = lib
TARGET = genesisx_orm
CONFIG += c++23

ios {
    CONFIG -= dll shared
    CONFIG += staticlib
} else:macos {
    CONFIG += shared staticlib
} else:win32 {
CONFIG += dll
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

# ORM links to core; search same central dir
QMAKE_LIBDIR += $$DESTDIR
android {
    LIBS += -lgenesisx_arm64-v8a
} else {
    LIBS += -lgenesisx
}

INCLUDEPATH += $$GENESISX_BUILD_ROOT/orm/include

HEADERS += $$files($$PWD/include/GenesisX/Orm/*.h) \
    # include/GenesisX/Orm/ApiClient.h \
    # include/GenesisX/Orm/AuthCredentials.h \
    # include/GenesisX/Orm/CommandController.h \
    # include/GenesisX/Orm/ConnectionCheck.h \
    # include/GenesisX/Orm/ConnectionController.h \
    # include/GenesisX/Orm/HttpConfig.h \
    # include/GenesisX/Orm/HttpResponse.h \
    # include/GenesisX/Orm/genesisx_orm_global.h

SOURCES += $$files($$PWD/src/core/*.cpp)\
    # src/core/ApiClient.cpp \
    # src/core/CommandController.cpp \
    # src/core/ConnectionController.cpp \
    # src/core/HttpConnectionChecker.cpp
