# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

TEMPLATE = subdirs
CONFIG += qt c++23
CONFIG += ordered

SUBDIRS += core physics orm

core.subdir = $$PWD/core

physics.subdir  = $$PWD/physics
physics.depends = core

orm.subdir = $$PWD/orm
orm.depends = core

DISTFILES += \
    README.md \
    BACKERS.md \
    LICENSE \
    LICENSING.md \
    CONTRIBUTING.md \
    CODE_OF_CONDUCT.md \
    SUPPORT.md \
    SECURITY.md \
    LICENSES/LicenseRef-KooijmanInc-Commercial.txt \
    LICENSES/GPL-3.0-only.txt \


SUBDIRS += zdev

# build physics too:
# qmake genesisx.pro "CONFIG+=genesisx_build_physics"

