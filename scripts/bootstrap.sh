#!/usr/bin/env bash
# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PKG_DIR="$ROOT/scripts/packages"

usage() {
  echo "Usage: $0 [all|list|<package> ...]"
  echo "Examples:"
  echo "  $0                 # same as 'all'"
  echo "  $0 all             # run every package script"
  echo "  $0 list            # show available packages"
  echo "  $0 firebase        # run one package"
  echo "  $0 firebase bullet # run multiple packages"
  exit 1
}

list_packages() {
  # show *.sh scripts as package names
#  find "$PKG_DIR" -maxdepth 1 -type f -name "*.sh" -printf "%f\n" 2>/dev/null \
#    | sed 's/\.sh$//' | sort
  find "$PKG_DIR" -type f -name "*.sh" 2>/dev/null \
    | while IFS= read -r path; do
      if [ "$(dirname "$path")" = "$PKG_DIR" ]; then
        fname=${path##*/}
        echo "${fname%.sh}"
      fi
    done | sort
}

run_pkg() {
  local name="$1"
  local script="$PKG_DIR/$name.sh"
  if [[ -x "$script" ]]; then
    echo "==> Running package: $name"
    "$script"
  else
    echo "No script for package: $name"
    return 1
  fi
}

# Parse args
if (( $# == 0 )); then set -- all; fi

case "${1:-}" in
  -h|--help)
    usage ;;
  list)
    list_packages
    exit 0
    ;;
  all)
    shift
    pkgs=( $(list_packages) )
    for p in "${pkgs[@]}"; do echo "$p"; run_pkg "$p"; done
    ;;
  *)
    # allow multiple explicit package names
    for arg in "$@"; do run_pkg "$arg"; done
    ;;
esac

