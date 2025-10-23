#!/usr/bin/env bash
# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.
set -euo pipefail

BASE="staging"
PREFIX='^chore/'
REMOTES=("origin" "gitlab")
DO_DELETE=0

usage() {
  cat <<EOF
Usage: $(basename "$0") [options]

Options:
  -b, --base <branch>      Base branch to check "merged into" (default: staging)
  -p, --prefix <regex>     Regex to select branch names (default: ^chore/)
  -r, --remote <list>      Comma-separated remotes (default: origin,gitlab)
      --delete             Perform deletion (default: dry-run)
  -n, --dry-run            Dry-run (explicit)
  -h, --help               Show this help
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    -b|--base)   BASE="$2"; shift 2;;
    -p|--prefix) PREFIX="$2"; shift 2;;
    -r|--remote) IFS=',' read -r -a REMOTES <<<"$2"; shift 2;;
    --delete)    DO_DELETE=1; shift;;
    -n|--dry-run)DO_DELETE=0; shift;;
    -h|--help)   usage; exit 0;;
    *) echo "Unknown arg: $1"; usage; exit 1;;
  esac
done

PROTECTED=("main" "master" "staging" "$BASE")

remote_exists() {
  git remote | grep -qx "$1"
}

cleanup_remote() {
  local remote="$1"
  if ! remote_exists "$remote"; then
    echo "== Remote: $remote =="
    echo "Skipping: remote not configured in this repo."
    return
  fi

  echo "== Remote: $remote =="
  git fetch "$remote" --prune

  # All branches already merged into remote/base
  local merged
  merged=$(git branch -r --merged "$remote/$BASE" | sed 's|^[ *]*||' | grep "^$remote/" || true)

  local targets=()
  while IFS= read -r line; do
    [[ -z "$line" ]] && continue
    local name="${line#${remote}/}"    # strip "<remote>/"
    [[ "$name" =~ $PREFIX ]] || continue
    for p in "${PROTECTED[@]}"; do [[ "$name" == "$p" ]] && continue 2; done
    targets+=("$name")
  done <<< "$merged"

  if ((${#targets[@]}==0)); then
    echo "No candidates on $remote (merged into $BASE & matching $PREFIX)."
    return
  fi

  echo "Candidates on $remote:"
  printf '  - %s\n' "${targets[@]}"

  if ((DO_DELETE)); then
    for b in "${targets[@]}"; do
      echo "Deleting $remote/$b"
      git push "$remote" --delete "$b" || echo "WARN: failed to delete $b on $remote"
    done
  else
    echo "(dry-run) Pass --delete to remove them."
  fi
}

for r in "${REMOTES[@]}"; do
  cleanup_remote "$r"
done
