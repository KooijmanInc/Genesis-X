#!/usr/bin/env bash
# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

set -euo pipefail

# Requirements: curl, unzip, jq, shasum (macOS) or sha256sum (Linux)
# macOS:  brew install jq
# Linux:  sudo apt-get install -y jq unzip (sha256sum is in coreutils)

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
DEST="$ROOT/3rdparty/firebase_cpp_sdk"
DEPS="$ROOT/config/deps.json"

# Read from deps.json
VER="$(jq -r '.firebase.version' "$DEPS")"
URL="$(jq -r '.firebase.url' "$DEPS")"
EXPECTED_SHA="$(jq -r '.firebase.sha256' "$DEPS")"

mkdir -p "$ROOT/3rdparty"

if [[ -d "$DEST" ]]; then
  echo "✅ firebase_cpp_sdk already present at: $DEST"
  exit 0
fi

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

ZIP="$tmp/firebase_cpp_sdk_${VER}.zip"

echo "⬇️  Downloading Firebase C++ SDK v$VER ..."
# Retry a few times, fail if HTTP status is not 2xx
curl -L --fail --retry 3 --retry-delay 2 -o "$ZIP" "$URL"

# Compute SHA-256 (if expected provided)
compute_sha256() {
  if command -v sha256sum >/dev/null 2>&1; then
    sha256sum "$1" | awk '{print $1}'
  else
    # macOS shasum
    shasum -a 256 "$1" | awk '{print $1}'
  fi
}

if [[ -n "${EXPECTED_SHA// /}" ]]; then
  echo "🔐 Verifying checksum..."
  ACTUAL_SHA="$(compute_sha256 "$ZIP")"
  if [[ "$ACTUAL_SHA" != "$EXPECTED_SHA" ]]; then
    echo "❌ SHA-256 mismatch!"
    echo "Expected: $EXPECTED_SHA"
    echo "Actual:   $ACTUAL_SHA"
    echo "Refusing to proceed."
    exit 1
  fi
  echo "✅ Checksum OK."
else
  echo "⚠️  No SHA-256 provided in deps.json. Skipping verification."
fi

echo "📦 Unpacking..."
unzip -q "$ZIP" -d "$tmp"

# Google’s zip expands to a folder named 'firebase_cpp_sdk'
if [[ ! -d "$tmp/firebase_cpp_sdk" ]]; then
  echo "❌ Expected folder 'firebase_cpp_sdk' not found after unzip."
  exit 1
fi

mv "$tmp/firebase_cpp_sdk" "$DEST"
echo "✅ Firebase C++ SDK v$VER ready at: $DEST"

