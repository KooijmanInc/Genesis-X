TEMPLATE = subdirs
CONFIG += qt c++23
CONFIG += ordered

SUBDIRS += core physics

core.subdir = $$PWD/core

physics.subdir  = $$PWD/physics
physics.depends = core

DISTFILES += \
    README.md \
    BACKERS.md \
    LICENSE \
    .github/FUNDING.yml \
    mkspecs/modules/qt_lib_genesisx.pri \
    mkspecs/modules/qt_lib_genesisx_physics.pri \
    mkspecs/features/gx_app_root.prf \
    .gitignore \
    config/deps.json \
    scripts/bootstrap.bat \
    scripts/bootstrap.sh \
    scripts/packages/firebase.bat \
    scripts/packages/firebase.sh \
    tools/update_gradle_props.ps1.in
    # mkspecs/modules/qt_lib_genesisx-physics.pri

# build physics too:
# qmake genesisx.pro "CONFIG+=genesisx_build_physics"
