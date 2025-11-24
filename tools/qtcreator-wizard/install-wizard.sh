#!/usr/bin/env bash
# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

set -e

echo "====================================="
echo " Genesis-X Qt wizard Installer"
echo "====================================="
echo

install_wizard() {
    local DEST_DIR=""

    if [ -n "$APPDATA" ]; then
        DEST_DIR="$APPDATA/QtProject/qtcreator/templates/wizards/"
    elif [ -d "/mnt/c" ]; then
        local WINUSER
        WINUSER="$(cmd.exe /c "echo %USERNAME%" 2>/dev/null | tr -d '\r')"

        if [ -z "$WINUSER" ]; then
            read -rp "   Windows username for Qt Creator (e.g. Bas): " WINUSER
        fi

        DEST_DIR="/mnt/c/Users/${WINUSER}/AppData/Roaming/QtProject/qtcreator/templates/wizards/"
    else
        DEST_DIR="$HOME/.config/QtProject/qtcreator/templates/wizards/"
    fi

    mkdir -p "$DEST_DIR" || {
        echo "❌ Could not create directory: $DEST_DIR"
        return 1
    }

    echo "Installing wizard to: $DEST_DIR"

    WIZARDS="$(pwd)/tools/qtcreator-wizard/projects"

    cp -r "$WIZARDS"/* "$DEST_DIR" || {
        echo "❌ Failed to copy wizards to: $DEST_DIR"
        return 1
    }

    echo "✅ Wizards installed:"
    echo "   $DEST_DIR"
    echo
}

install_wizard
