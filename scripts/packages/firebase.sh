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

if [ -d "/mnt/c" ]; then
  JQ_BIN="$ROOT/tools/jq-windows-amd64.exe"
  DEPS="$(wslpath -w "$DEPS")"
else
  JQ_BIN="jq"
fi

if ! $JQ_BIN -V >/dev/null 2>&1; then
  echo "❌ 'jq' is required but not found."
  echo "   Please install jq or add it to PATH."
  echo "   - Windows (Qt terminal): put jq.exe somewhere and add it to PATH,"
  echo "     or into tools/jq/jq.exe and adjust this script."
  echo "   - Linux:  sudo apt-get install jq"
  echo "   - macOS:  brew install jq"
  exit 1
fi

#read_json() {
#  local filter="${1:-}"
#  if [ -z "$filter" ]; then
#    echo "read_json: missing jq filter" >&2
#    exit 1
#  fi

#  "$JQ_BIN" -r "$filter" "$DEPS" | tr -d '\r' | tr -d '\n'
#}

#read_json

# Read from deps.json
VER="$($JQ_BIN -r '.firebase.version' "$DEPS" | tr -d '\r' | tr -d '\n')"
URL="$($JQ_BIN -r '.firebase.url' "$DEPS" | tr -d '\r' | tr -d '\n')"
EXPECTED_SHA="$($JQ_BIN -r '.firebase.sha256' "$DEPS" | tr -d '\r' | tr -d '\n')"

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

echo "📦 Unpacking (this can take several minutes since average size is 8GB)..."
unzip -q "$ZIP" -d "$tmp" &
unzip_pid=$!

# Simple spinner
spinner='|/-\'
i=0

# Spinner loop
while kill -0 "$unzip_pid" 2>/dev/null; do
  i=$(( (i + 1) % 4 ))
  printf "\r📦 Unpacking... %s" "${spinner:$i:1}"
  sleep 0.2
done

# Make sure we get the real exit code
wait "$unzip_pid"
unzip_status=$?

printf "\r"  # clear spinner line

if [ $unzip_status -ne 0 ]; then
  echo "❌ unzip failed with status $unzip_status"
  exit $unzip_status
fi

echo "✅ Unpacking complete."

# Google’s zip expands to a folder named 'firebase_cpp_sdk'
if [[ ! -d "$tmp/firebase_cpp_sdk" ]]; then
  echo "❌ Expected folder 'firebase_cpp_sdk' not found after unzip."
  exit 1
fi

echo "🚚 Moving Firebase C++ SDK into place (this can take a moment)..."

if [[ -d "$DEST" ]]; then
  echo "✅ firebase_cpp_sdk already present at: $DEST"
  exit 0
fi

mv "$tmp/firebase_cpp_sdk" "$DEST"
#mv_pid=$!

#spinner='|/-\'
#i=0

## Spinner while mv is running
#while kill -0 "$mv_pid" 2>/dev/null; do
#    i=$(( (i + 1) % 4 ))
#    printf "\r🚚 Moving... %s" "${spinner:$i:1}"
#    sleep 0.2
#done

## Wait for mv to finish and capture status
#wait "$mv_pid"
#mv_status=$?

## Clear spinner line
#printf "\r"

#if [ $mv_status -ne 0 ]; then
#    echo "❌ Moving Firebase C++ SDK failed (exit code $mv_status)"
#    exit $mv_status
#fi
echo "✅ Firebase C++ SDK v$VER ready at: $DEST"

