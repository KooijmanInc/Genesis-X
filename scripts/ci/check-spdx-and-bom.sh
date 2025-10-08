#!/usr/bin/env bash
set -euo pipefail

ROOT="$(git rev-parse --show-toplevel 2>/dev/null || true)"
if [[ -z "$ROOT" ]]; then
  echo "Run inside a Git repository." >&2
  exit 1
fi
cd "$ROOT"

declare -a FILES missing bom

# Gather all tracked files
while IFS= read -r -d '' f; do
  FILES+=("$f")
done < <(git ls-files -z)

# Exclusions
EXCLUDE_REGEX='(^LICENSE(S)?/)|(\.gitignore$)|(\.gitattributes$)|(.*firebase_cpp_sdk/)|(.*node_modules/)|(.*build/)|(.*dist/)'

# Extensions we expect to support SPDX headers
SPDX_EXTS='^(c|cc|cxx|cpp|h|hh|hpp|qml|js|ts|java|kt|m|mm|gradle|groovy|kts|sh|bash|ps1|py|rb|pl|pro|pri|yml|yaml|toml|ini|cfg|conf|properties|cmake|xml|qrc|ui|plist|html|htm|md|markdown|bat|cmd)$'

# Files sensitive to UTF-8 BOM (qmake/Gradle/etc.)
BOM_SENSITIVE='^(pro|pri|qml|qrc|ui|gradle|kts|groovy|sh|ps1|bat|cmd|yml|yaml|xml|md|txt)$'

is_text() { grep -Iq . "$1"; }

for f in "${FILES[@]}"; do
  [[ "$f" =~ $EXCLUDE_REGEX ]] && continue
  if is_text "$f"; then
    base="$(basename "$f")"
    ext="${f##*.}"

    # SPDX check (first 5 lines is enough for a header)
    if [[ "$base" == "CMakeLists.txt" || "$ext" =~ $SPDX_EXTS ]]; then
      if ! head -n 5 "$f" | grep -q "SPDX-License-Identifier"; then
        missing+=("$f")
      fi
    fi

    # BOM check
    if [[ "$ext" =~ $BOM_SENSITIVE ]]; then
      if [ -s "$f" ]; then
        sig=$(head -c 3 "$f" | od -An -t x1 | tr -d ' \n')
        if [[ "$sig" == "efbbbf" ]]; then
          bom+=("$f")
        fi
      fi
    fi
  fi
done

rc=0
if ((${#missing[@]})); then
  echo "❌ Files missing SPDX header:"
  printf ' - %s\n' "${missing[@]}"
  rc=1
fi
if ((${#bom[@]})); then
  echo "❌ Files contain UTF-8 BOM (not allowed):"
  printf ' - %s\n' "${bom[@]}"
  rc=1
fi

if (( rc == 0 )); then
  echo "✅ SPDX & BOM checks passed."
fi

exit $rc
