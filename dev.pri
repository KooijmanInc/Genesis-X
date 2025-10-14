# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

# contains(CONFIG, dev) {
#     DOCS  = $$files($$PWD/docs/*, true)
#     TOOLS = $$files($$PWD/tools/*, true)
#     DISTFILES += $$DOCS
#     DISTFILES += $$TOOLS
#     DISTFILES += \
#         .github/FUNDING.yml \
#         .github/PULL_REQUEST_TEMPLATE.md \
#         .github/ISSUE_TEMPLATE/bug_report.md \
#         .github/ISSUE_TEMPLATE/feature_request.md \
#         .github/ISSUE_TEMPLATE/config.yml \
#         .github/workflows/quality.yml \
#         .gitlab-ci.yml \
#         mkspecs/modules/qt_lib_genesisx.pri \
#         mkspecs/modules/qt_lib_genesisx_physics.pri \
#         mkspecs/features/gx_app_root.prf \
#         .gitignore \
#         config/deps.json \
#         scripts/bootstrap.bat \
#         scripts/bootstrap.sh \
#         scripts/release.bat \
#         scripts/release.sh \
#         scripts/add-headers.sh \
#         scripts/add-headers.ps1 \
#         scripts/add-headers.bat \
#         scripts/cleanup-merged-branches.sh \
#         scripts/cleanup-merged-branches.ps1 \
#         scripts/cleanup-merged-branches.bat \
#         scripts/fetch-gpl-license.sh \
#         scripts/fetch-gpl-license.ps1 \
#         scripts/fetch-gpl-license.bat \
#         scripts/packages/firebase.bat \
#         scripts/packages/firebase.sh \
#         scripts/ci/check-spdx-and-bom.sh \
#         scripts/ci/check-spdx-and-bom.ps1 \
#         scripts/ci/check-spdx-and-bom.bat
# } else {
#     DISTFILES -= $$DOCS
#     DISTFILES -= $$TOOLS
# }

