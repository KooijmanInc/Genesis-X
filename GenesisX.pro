# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

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
    LICENSING.md \
    CONTRIBUTING.md \
    CODE_OF_CONDUCT.md \
    SUPPORT.md \
    SECURITY.md \
    LICENSES/LicenseRef-KooijmanInc-Commercial.txt \
    LICENSES/GPL-3.0-only.txt \
    .github/FUNDING.yml \
    .github/PULL_REQUEST_TEMPLATE.md \
    .github/ISSUE_TEMPLATE/bug_report.md \
    .github/ISSUE_TEMPLATE/feature_request.md \
    .github/ISSUE_TEMPLATE/config.yml \
    .github/workflows/quality.yml \
    .gitlab-ci.yml \
    mkspecs/modules/qt_lib_genesisx.pri \
    mkspecs/modules/qt_lib_genesisx_physics.pri \
    mkspecs/features/gx_app_root.prf \
    .gitignore \
    config/deps.json \
    scripts/bootstrap.bat \
    scripts/bootstrap.sh \
    scripts/release.bat \
    scripts/release.sh \
    scripts/add-headers.sh \
    scripts/add-headers.ps1 \
    scripts/add-headers.bat \
    scripts/cleanup-merged-branches.sh \
    scripts/cleanup-merged-branches.ps1 \
    scripts/cleanup-merged-branches.bat \
    scripts/fetch-gpl-license.sh \
    scripts/fetch-gpl-license.ps1 \
    scripts/fetch-gpl-license.bat \
    scripts/packages/firebase.bat \
    scripts/packages/firebase.sh \
    scripts/ci/check-spdx-and-bom.sh \
    scripts/ci/check-spdx-and-bom.ps1 \
    scripts/ci/check-spdx-and-bom.bat \
    tools/update_gradle_props.ps1.in
    # mkspecs/modules/qt_lib_genesisx-physics.pri

# build physics too:
# qmake genesisx.pro "CONFIG+=genesisx_build_physics"

