QT += core
TEMPLATE = lib
QT      += core
CONFIG  += plugin c++23
TARGET = gxplugin_physics

DESTDIR  = $$OUT_PWD/../../plugins

# dynamic everywhere except iOS, where plugins must be static
ios: {
    CONFIG += staticlib
    DEFINES += GX_STATIC_PLUGIN
}

INCLUDEPATH += $$PWD/.. $$PWD/../../core/include $$PWD/../include
SOURCES += $$PWD/physics_plugin.cpp \
    physics_plugin.cpp

DESTDIR  = $$GENESISX_BUILD_ROOT/plugins/$$PLATFORM_PATH/$$COMPILER_PATH/$$PROCESSOR_PATH/$$GX_CFG

QMAKE_LIBDIR += $$GENESISX_OUT_BIN
LIBS += -lgenesisx_physics -lgenesisx
