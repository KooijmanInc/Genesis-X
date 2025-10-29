#!/usr/bin/env bash
# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

set -euo pipefail
# Add SPDX + copyright headers to tracked source files.
# Customize via env:
#   COPYRIGHT_OWNER="Kooijman Incorporate Holding B.V."
#   COPYRIGHT_YEAR="2025" (default: current year)
#   LICENSE_EXPR="(LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)"

ROOT="$(git rev-parse --show-toplevel 2>/dev/null || true)"
if [[ -z "$ROOT" ]]; then
  echo "Run inside a Git repository."; exit 1
fi
cd "$ROOT"

COPYRIGHT_OWNER="${COPYRIGHT_OWNER:-Kooijman Incorporate Holding B.V.}"
COPYRIGHT_YEAR="${COPYRIGHT_YEAR:-2025}"
LICENSE_EXPR="${LICENSE_EXPR:-(LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)}"

is_text() { grep -qI . "$1"; }

comment_style() {  # echo one of: slash hash bat angle none
  local f="$1"; local base ext
  base="$(basename "$f")"
  ext="${f##*.}"
  shopt -s nocasematch
  case "$base" in
    CMakeLists.txt) echo hash; return;;
  esac
  case "$ext" in
    c|cc|cxx|cpp|h|hh|hpp|qml|js|ts|java|kt|m|mm|gradle|groovy|kts) echo slash;;
    sh|bash|ps1|py|rb|pl|pro|pri|yml|yaml|toml|ini|cfg|conf|properties|cmake) echo hash;;
    bat|cmd) echo bat;;
    xml|qrc|ui|plist|html|htm|md|markdown) echo angle;;
    json|jpg|jpeg|png|gif|svg|ico|pdf|zip|gz|7z|tar|rar|a|so|dll|dylib|bin) echo none;;
    *) echo none;;
  esac
}

make_header() {
  local style="$1"
  case "$style" in
    slash)
      printf "// SPDX-License-Identifier: %s\n// Copyright (c) %s %s\n\n" "$LICENSE_EXPR" "$COPYRIGHT_YEAR" "$COPYRIGHT_OWNER";;
    hash)
      printf "# SPDX-License-Identifier: %s\n# Copyright (c) %s %s\n\n" "$LICENSE_EXPR" "$COPYRIGHT_YEAR" "$COPYRIGHT_OWNER";;
    bat)
      printf ":: SPDX-License-Identifier: %s\n:: Copyright (c) %s %s\n\n" "$LICENSE_EXPR" "$COPYRIGHT_YEAR" "$COPYRIGHT_OWNER";;
    angle)
      printf "<!-- SPDX-License-Identifier: %s -->\n<!-- Copyright (c) %s %s -->\n\n" "$LICENSE_EXPR" "$COPYRIGHT_YEAR" "$COPYRIGHT_OWNER";;
  esac
}

process_file() {
  local f="$1"
  local style="$(comment_style "$f")"
  [[ "$style" == "none" ]] && return 0

  # Skip if header already present
  if head -n 5 "$f" | grep -q "SPDX-License-Identifier"; then
    return 0
  fi

  local tmp="$f.$$"
  local first="$(head -n 1 "$f" || true)"

  if [[ "$first" =~ ^#! ]]; then
    printf "%s\n" "$first" > "$tmp"
    make_header "$style" >> "$tmp"
    tail -n +2 "$f" >> "$tmp"
  else
    make_header "$style" > "$tmp"
    cat "$f" >> "$tmp"
  fi
  mv "$tmp" "$f"
  echo "Header added: $f"
}

# Tracked files only; exclude license files and common generated/vendor dirs
mapfile -d '' FILES < <(git ls-files -z   ':!LICENSE' ':!LICENSE.*' ':!NOTICE' ':!NOTICE.*' ':!docs' ':!docs.*'   ':!LICENSES/**' ':!**/*.json' ':!.gitignore' ':!.gitattributes'   ':!**/firebase_cpp_sdk/**' ':!**/node_modules/**' ':!**/build/**' ':!**/dist/**' )

for f in "${FILES[@]}"; do
  if is_text "$f"; then
    process_file "$f" || true
  fi
done

echo "Done. Override: COPYRIGHT_OWNER, COPYRIGHT_YEAR, LICENSE_EXPR"
