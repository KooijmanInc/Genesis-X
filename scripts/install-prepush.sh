#!/usr/bin/env bash
# Install the pre-push hook into a repo-local hooks directory (.githooks)
set -euo pipefail
ROOT="$(git rev-parse --show-toplevel)"
HOOKS="$ROOT/.githooks"
SRC_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

mkdir -p "$HOOKS"
cp -f "$SRC_DIR/pre-push" "$HOOKS/pre-push"
chmod +x "$HOOKS/pre-push"

git config core.hooksPath .githooks

echo "Installed pre-push hook to .githooks/pre-push"
echo "Protected branches: ${PROTECTED_BRANCHES:-main staging}"
echo "Override once with: ALLOW_PROTECTED_PUSH=1 git push <remote> <branch>"
