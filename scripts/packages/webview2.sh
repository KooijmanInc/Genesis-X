#!/usr/bin/env bash
# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

set -euo pipefail

# Requirements: curl, unzip, jq, shasum (macOS) or sha256sum (Linux)
# macOS:  brew install jq
# Linux:  sudo apt-get install -y jq unzip (sha256sum is in coreutils)

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
DEST="$ROOT/3rdparty/webview2"
#DEPS="$ROOT/config/deps.json"
TMP="$(mktemp -d)"

# You can pin a known-good version here later.
# If empty, we grab the latest package version from NuGet flat-container.
PIN_VERSION="${WEBVIEW2_VERSION:-}"

need_cmd() {
  command -v "$1" >/dev/null 2>&1 || {
    echo "❌ Missing required tool: $1"
    echo "   Please install it and re-run."
    exit 1
  }
}

need_cmd curl
need_cmd unzip

echo "==> Installing WebView2 SDK assets into: $DEST"

# Resolve latest version if not pinned
if [[ -z "$PIN_VERSION" ]]; then
  echo "==> Resolving latest Microsoft.Web.WebView2 version from NuGet..."
  # NuGet flat container: .../index.json contains versions array
  INDEX_JSON="$TMP/index.json"
  curl -fsSL \
    "https://api.nuget.org/v3-flatcontainer/microsoft.web.webview2/index.json" \
    -o "$INDEX_JSON"

  # crude but effective: take the last "version" entry
  PIN_VERSION="$(grep -oE '"[0-9]+\.[0-9]+\.[0-9]+(\.[0-9]+)?(-[a-z0-9.-]+)?"' "$INDEX_JSON" | tr -d '"' | tail -n1)"
  if [[ -z "$PIN_VERSION" ]]; then
    echo "❌ Could not resolve WebView2 package version."
    exit 1
  fi
fi

echo "==> Using Microsoft.Web.WebView2 version: $PIN_VERSION"

PKG_URL="https://api.nuget.org/v3-flatcontainer/microsoft.web.webview2/${PIN_VERSION}/microsoft.web.webview2.${PIN_VERSION}.nupkg"
NUPKG="$TMP/webview2.nupkg"

echo "==> Downloading: $PKG_URL"
curl -fsSL "$PKG_URL" -o "$NUPKG"

echo "==> Extracting..."
unzip -q "$NUPKG" -d "$TMP/pkg"

# Create destination folders
mkdir -p "$DEST/include" "$DEST/loader"

# Copy headers (the package includes the native headers under build/native/include)
if compgen -G "$TMP/pkg/build/native/include/*.h" > /dev/null; then
  cp -f "$TMP/pkg/build/native/include/"*.h "$DEST/include/"
else
  echo "❌ Could not find headers under build/native/include in the nupkg."
  echo "   The package layout may have changed."
  exit 1
fi

# Copy loader DLL (common locations are 'content' and/or contentFiles; we search)
LOADER_DLL="$(find "$TMP/pkg" -type f -iname "WebView2Loader.dll" | head -n1 || true)"
if [[ -z "$LOADER_DLL" ]]; then
  echo "❌ Could not find WebView2Loader.dll in the nupkg."
  echo "   The package layout may have changed."
  exit 1
fi

cp -f "$LOADER_DLL" "$DEST/loader/WebView2Loader.dll"

echo "✅ WebView2 SDK assets installed:"
echo "   Headers : $DEST/include"
echo "   Loader  : $DEST/loader/WebView2Loader.dll"
echo
echo "Tip: You can pin a version by running:"
echo "  WEBVIEW2_VERSION=$PIN_VERSION $ROOT/scripts/bootstrap.sh webview2"

rm -rf "$TMP"
