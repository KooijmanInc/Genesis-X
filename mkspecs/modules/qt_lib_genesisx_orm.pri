# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

# ---- GenesisX ORM qmake module ----
QT.genesisx_orm.name          = genesisx_orm
QT.genesisx_orm.depends       = network
QT.genesisx_orm.friendly_name = GenesisX (Orm)

GENESISX_BUILD_ROOT = $$clean_path($$PWD/../..)
include($$GENESISX_BUILD_ROOT/common/qmake-target-platform.pri)
include($$GENESISX_BUILD_ROOT/common/qmake-destination-path.pri)

QML_IMPORT_PATH += $$GENESISX_BUILD_ROOT/orm/qml
