#!/usr/bin/env bash
# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

set -euo pipefail
ver="${1:-}"; msg="${2:-}"
[[ -z "$ver" || -z "$msg" ]] && { echo "usage: $0 vX.Y.Z \"message\""; exit 1; }
git diff --quiet || { echo "Working tree not clean"; exit 1; }
git fetch --tags
git tag -a "$ver" -m "$msg"
git push origin "$ver"
echo "Tag $ver pushed to origin; mirror will sync to GitHub."

