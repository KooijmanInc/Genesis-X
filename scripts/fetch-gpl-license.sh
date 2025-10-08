#!/usr/bin/env bash
# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

set -euo pipefail
mkdir -p LICENSES
curl -fsSL https://www.gnu.org/licenses/gpl-3.0.txt -o LICENSES/GPL-3.0-only.txt
echo "Downloaded GPL-3.0-only license text to LICENSES/GPL-3.0-only.txt"
