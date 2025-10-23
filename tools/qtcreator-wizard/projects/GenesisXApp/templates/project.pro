# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

# Genesis-X skeleton
TEMPLATE = subdirs

SUBDIRS += \\
    %{UiDir}

DISTFILES += \\
    .gitignore \\
    .qmake.conf \\
    features/gx_app_root.prf
