#!/usr/bin/env bash
# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

set -euo pipefail

# Requirements: curl, unzip, jq, shasum (macOS) or sha256sum (Linux)
# macOS:  brew install jq
# Linux:  sudo apt-get install -y jq unzip (sha256sum is in coreutils)

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
DEST_CPP="$ROOT/3rdparty/firebase_cpp_sdk"
DEST_IOS="$ROOT/3rdparty/firebase_ios_sdk"
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
VER_CPP="$($JQ_BIN -r '.firebase.version' "$DEPS" | tr -d '\r' | tr -d '\n')"
URL_CPP="$($JQ_BIN -r '.firebase.url' "$DEPS" | tr -d '\r' | tr -d '\n')"
EXPECTED_SHA_CPP="$($JQ_BIN -r '.firebase.sha256' "$DEPS" | tr -d '\r' | tr -d '\n')"

VER_IOS="$($JQ_BIN -r '.firebase_ios.version' "$DEPS" | tr -d '\r' | tr -d '\n')"
URL_IOS="$($JQ_BIN -r '.firebase_ios.url' "$DEPS" | tr -d '\r' | tr -d '\n')"
EXPECTED_SHA_IOS="$($JQ_BIN -r '.firebase_ios.sha256' "$DEPS" | tr -d '\r' | tr -d '\n')"

mkdir -p "$ROOT/3rdparty"

compute_sha256() {
  if command -v sha256sum >/dev/null 2>&1; then
    sha256sum "$1" | awk '{print $1}'
  else
    # macOS shasum
    shasum -a 256 "$1" | awk '{print $1}'
  fi
}

########################################
# 1) Install Firebase C++ SDK (desktop)
########################################

if [[ -d "$DEST_CPP" ]]; then
  echo "✅ firebase_cpp_sdk already present at: $DEST_CPP"
else
  tmp="$(mktemp -d)"
  trap 'rm -rf "$tmp"' EXIT

  ZIP_CPP="$tmp/firebase_cpp_sdk_${VER_CPP}.zip"

  echo "⬇️  Downloading Firebase C++ SDK v$VER_CPP ..."
  # Retry a few times, fail if HTTP status is not 2xx
  curl -L --fail --retry 3 --retry-delay 2 -o "$ZIP_CPP" "$URL_CPP"

  if [[ -n "${EXPECTED_SHA_CPP// /}" ]]; then
    echo "🔐 Verifying checksum..."
    ACTUAL_SHA_CPP="$(compute_sha256 "$ZIP_CPP")"
    if [[ "$ACTUAL_SHA_CPP" != "$EXPECTED_SHA_CPP" ]]; then
      echo "❌ SHA-256 mismatch!"
      echo "Expected: $EXPECTED_SHA_CPP"
      echo "Actual:   $ACTUAL_SHA_CPP"
      echo "Refusing to proceed."
      exit 1
    fi
    echo "✅ Checksum OK."
  else
    echo "⚠️  No SHA-256 provided in deps.json. Skipping verification."
  fi

  echo "📦 Unpacking (this can take several minutes since average size is 8GB)..."
  unzip -q "$ZIP_CPP" -d "$tmp" &
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

  if [[ -d "$DEST_CPP" ]]; then
    echo "✅ firebase_cpp_sdk already present at: $DEST_CPP"
    exit 0
  fi

  mv "$tmp/firebase_cpp_sdk" "$DEST_CPP"

  rm -rf "$tmp"
  trap - EXIT

  echo "✅ Firebase C++ SDK v$VER_CPP ready at: $DEST_CPP"
fi

##############################################
# 2) Install Firebase iOS/Apple SDK (desktop)
##############################################

if [[ -d "$DEST_IOS" ]]; then
  echo "✅ firebase_ios_sdk already present at: $DEST_IOS"
  exit 0
else
  tmp="$(mktemp -d)"
  trap 'rm -rf "$tmp"' EXIT

  ZIP_IOS="$tmp/firebase_ios_sdk_${VER_IOS}.zip"

  echo "⬇️  Downloading Firebase iOS SDK v$VER_IOS ..."
  # Retry a few times, fail if HTTP status is not 2xx
  curl -L --fail --retry 3 --retry-delay 2 -o "$ZIP_IOS" "$URL_IOS"

  if [[ -n "${EXPECTED_SHA_IOS// /}" ]]; then
    echo "🔐 Verifying checksum..."
    ACTUAL_SHA_IOS="$(compute_sha256 "$ZIP_IOS")"
    if [[ "$ACTUAL_SHA_IOS" != "$EXPECTED_SHA_IOS" ]]; then
      echo "❌ SHA-256 mismatch!"
      echo "Expected: $EXPECTED_SHA_IOS"
      echo "Actual:   $ACTUAL_SHA_IOS"
      echo "Refusing to proceed."
      exit 1
    fi
    echo "✅ Checksum OK."
  else
    echo "⚠️  No SHA-256 provided in deps.json. Skipping verification."
  fi

  echo "📦 Unpacking Firebase iOS SDK (this can take several minutes since average size is 8GB)..."
  unzip -q "$ZIP_IOS" -d "$tmp" &
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
FF_OTHER=no
  # Google’s zip expands to a folder named 'firebase_ios_sdk'
  if [[ ! -d "$tmp/Firebase" ]]; then
    echo "❌ Expected folder 'firebase_ios_sdk' not found after unzip."
    echo "  Contents are:"
    ls -la "$tmp"
#    if [[ ! -d "$tmp/11_15_0" ]]; then
#      echo "❌ Expected folder '11_15_0' not found after unzip."
#      echo "  Contents are:"
#      ls -la "$tmp"
#      exit 1
#    else
#      mv "$tmp/11_15_0" "$DEST_IOS"
#    fi
    exit 1
  fi

  echo "🚚 Moving Firebase iOS SDK into place (this can take a moment)..."

  if [[ -d "$DEST_IOS" ]]; then
    echo "✅ firebase_ios_sdk already present at: $DEST_IOS"
    exit 0
  fi

  mv "$tmp/Firebase" "$DEST_IOS"

  rm -rf "$tmp"
  trap - EXIT

  echo "✅ Firebase iOS SDK v$VER_IOS ready at: $DEST_IOS"
fi
