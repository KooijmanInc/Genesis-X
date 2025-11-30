#!/usr/bin/env bash
# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.
set -euo pipefail

ROOT="$(git rev-parse --show-toplevel 2>/dev/null || true)"
if [[ -z "$ROOT" ]]; then
  echo "Run inside a Git repository." >&2
  exit 1
fi
cd "$ROOT"

# Explicit initialisation for nounset safety
FILES=()
missing=()
bom=()

# Gather all tracked files
while IFS= read -r -d '' f; do
  FILES+=("$f")
done < <(git ls-files -z)

# Exclusions
EXCLUDE_REGEX_PARTS=(
  '^LICENSE(S)?/'      # top-level LICENSE/ or LICENSES/
  '\.gitignore$'
  '\.gitattributes$'
  '.*firebase_cpp_sdk/'
  '.*node_modules/'
  '.*build/'
  '.*dist/'
  '.*templates/'       # templates (your new excludes)
  '.*scaffolds/'
  '.*examples/'
  '.*3rdparty/'
  '.*qtcreator-wizard/projects/'
  '.*docs/out/'
  '.*docs/debug/'
  '.*docs/release/'
  '.*docs/profile/'
  '.*docs/\.qdoccache/'
  '.*docs/.*\.qch$'
  '.*docs/.*\.qhp$'
  '.*docs/.*\.index$'
  '.*docs/\.qmake\.stash$'
  '.*docs/Makefile$'
  '.*docs/Makefile\.Debug$'
  '.*docs/Makefile\.Release$'
  '.*docs/Makefile\.Profile$'
  '.*tools/'
  '.*installer/'
)
EXCLUDE_REGEX="$(IFS='|'; echo "${EXCLUDE_REGEX_PARTS[*]}")"

#EXCLUDE_REGEX='(^LICENSE(S)?/)|(\.gitignore$)|(\.gitattributes$)|(.*firebase_cpp_sdk/)|(.*node_modules/)|(.*build/)|(.*dist/)'

# Extensions that should carry SPDX headers
SPDX_EXTS='^(c|cc|cxx|cpp|h|hh|hpp|qml|js|ts|java|kt|m|mm|gradle|groovy|kts|sh|bash|ps1|py|rb|pl|pro|pri|yml|yaml|toml|ini|cfg|conf|properties|cmake|xml|qrc|ui|plist|html|htm|md|markdown|bat|cmd)$'

# Files sensitive to UTF-8 BOM
BOM_SENSITIVE='^(pro|pri|qml|qrc|ui|gradle|kts|groovy|sh|ps1|bat|cmd|yml|yaml|xml|md|txt)$'

is_text() { grep -Iq . "$1"; }

for f in "${FILES[@]}"; do
  [[ "$f" =~ $EXCLUDE_REGEX ]] && continue
  if is_text "$f"; then
    base="$(basename "$f")"
    ext="${f##*.}"

    # SPDX check (look at first few lines)
    if [[ "$base" == "CMakeLists.txt" || "$ext" =~ $SPDX_EXTS ]]; then
      if ! head -n 5 "$f" | grep -q "SPDX-License-Identifier"; then
        missing+=("$f")
      fi
    fi

    # BOM check
    if [[ "$ext" =~ $BOM_SENSITIVE ]]; then
      if [ -s "$f" ]; then
        sig=$(head -c 3 "$f" | od -An -t x1 | tr -d ' \n')
        [[ "$sig" == "efbbbf" ]] && bom+=("$f")
      fi
    fi
  fi
done

rc=0
if (( ${#missing[@]} )); then
  echo "❌ Files missing SPDX header:"
  printf ' - %s\n' "${missing[@]}"
  rc=1
fi
if (( ${#bom[@]} )); then
  echo "❌ Files contain UTF-8 BOM (not allowed):"
  printf ' - %s\n' "${bom[@]}"
  rc=1
fi

if (( rc == 0 )); then
  echo "✅ SPDX & BOM checks passed."
fi
exit $rc
