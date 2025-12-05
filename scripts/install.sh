#!/usr/bin/env bash
# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

set -e

# Colors
RED="\033[31m"
GREEN="\033[32m"
YELLOW="\033[33m"
BLUE="\033[34m"
MAGENTA="\033[35m"
CYAN="\033[36m"
BOLD="\033[1m"
RESET="\033[0m"

echo "====================================="
echo " Genesis-X Library Installer"
echo "====================================="
echo
echo "Checking Qt environment..."
echo

QMAKE_CMD="null"
if qmake.exe -v >/dev/null 2>&1; then
    QMAKE_CMD="qmake.exe"
elif qmake6.exe -v >/dev/null 2>&1; then
    QMAKE_CMD="qmake6.exe"
elif qmake -v >/dev/null 2>&1; then
    QMAKE_CMD="qmake"
elif qmake6 -v >/dev/null 2>&1; then
    QMAKE_CMD="qmake6"
else
    echo -e "${RED}❌ Could not run qmake or qmake6.${RESET}"
    echo
    echo "Please run this installer from a Qt terminal, for example:"
    echo "  • Windows: 'Qt 6.x.x for MinGW 64-bit' Command Prompt"
    echo "  • macOS/Linux: a terminal where Qt's bin directory is on PATH"
    exit 1
fi

detect_qmake() {
    # 1) Explicit override
    if [ -n "${QMAKE_BIN:-}" ]; then
        if [ -x "$QMAKE_BIN" ]; then
            echo "$QMAKE_BIN"
            return 0
        else
            echo "Warning: QMAKE_BIN is set but not executable: $QMAKE_BIN" >&2
        fi
    fi

    # Helper to check "is this qmake a Qt 6 qmake?"
    is_qt6_qmake() {
        QT_VER="$("$1" -v 2>/dev/null | grep -i 'Qt version' | awk '{print $4}')"
        # Expect something like 6.10.0
        case "$QT_VER" in
            6.*) return 0 ;;
            *)   return 1 ;;
        esac
    }

    # 2) Try common Qt6 names first
    for CAND in qmake6 qmake-qt6; do
        if command -v "$CAND" >/dev/null 2>&1; then
            if is_qt6_qmake "$(command -v "$CAND")"; then
                command -v "$CAND"
                return 0
            fi
        fi
    done

    # 3) Try plain qmake but only accept Qt 6
    if command -v qmake >/dev/null 2>&1; then
        Q="$(command -v qmake)"
        if is_qt6_qmake "$Q"; then
            echo "$Q"
            return 0
        fi
    fi

    # 4) Look in typical Qt installer directories (Qt Online Installer)
    for base in "$HOME/Qt" "/opt/Qt"; do
        if [ -d "$base" ]; then
            CANDIDATE="$(find "$base" -maxdepth 4 -type f -name qmake 2>/dev/null | head -n1)"
            if [ -n "$CANDIDATE" ]; then
                if is_qt6_qmake "$CANDIDATE"; then
                    echo "$CANDIDATE"
                    return 0
                fi
            fi
        fi
    done

    return 1
}

if mingw32-make.exe -v >/dev/null 2>&1; then
    MAKE_CMD="mingw32-make.exe"
elif mingw32-make -v >/dev/null 2>&1; then
    MAKE_CMD="mingw32-make"
else
    MAKE_CMD="make"
fi

if [[ $QMAKE_CMD == "null" ]]; then
    QMAKE_BIN="$(detect_qmake || echo '')"

    if [ -z "$QMAKE_BIN" ]; then
        echo "Error: Could not find a Qt 6 'qmake' on this system." >&2
        echo "" >&2
        echo "Please either:" >&2
        echo "  - Install Qt 6 and ensure its qmake is in PATH," >&2
        echo "  - Or run this script with an explicit QMAKE_BIN, for example:" >&2
        echo "      QMAKE_BIN=\$HOME/Qt/6.10.0/gcc_64/bin/qmake ./scripts/install.sh" >&2
        exit 1
    else
    #    echo "Using qmake: $QMAKE_BIN"
        QMAKE_CMD=$QMAKE_BIN
    fi
fi

echo "→ Qt detected:"
echo "   qmake command : $QMAKE_CMD"
echo "   compile command : $MAKE_CMD"
echo

echo
echo "Detected current Genesis-X directory:"
CURRENT_DIR="$(pwd)"
echo "  $CURRENT_DIR"
echo
read -rp "Use this as the target project? [Y/n]" USE_CURRENT

TARGET=""

# If empty, use current dir
if [ -z "$USE_CURRENT" ] || [[ "$USE_CURRENT" =~ ^[Yy]$ ]]; then
    TARGET="$CURRENT_DIR"
fi

if [ ! -d "$TARGET" ]; then
    echo -e "${RED}❌ Install Genesis-X Aborting.${RESET}"
    exit 1
fi

echo
echo "What do you want to install?"
echo "  a) Install ALL features"
echo "  c) Custom selection"
echo "  q) Quit"
read -rp "Choose [a/c/q]: " CHOICE

FEATURES=()
HAS_DOCS=0
HAS_SNIPPET=0
HAS_WIZARD=0
HAS_THIRDPARTY=0

case "$CHOICE" in
    a|A)
        FEATURES=("docs" "snippets" "wizard" "thirdparty")
        ;;
    c|C)
        echo
        read -rp "Install documents? [Y/n]: " ANS
        [[ "$ANS" == "y" || "$ANS" == "Y" ]] && FEATURES+=("docs") && HAS_DOCS=1

        read -rp "Install snippets? (qmake autocomplete helper) [Y/n]: " ANS
        [[ "$ANS" == "y" || "$ANS" == "Y" ]] && FEATURES+=("snippets") && HAS_SNIPPET=1

        read -rp "Install wizard templates? [Y/n]: " ANS
        [[ "$ANS" == "y" || "$ANS" == "Y" ]] && FEATURES+=("wizard") && HAS_WIZARD=1

        read -rp "Install 3rdparty e.g. firebase? [Y/n]: " ANS
        [[ "$ANS" == "y" || "$ANS" == "Y" ]] && FEATURES+=("thirdparty") && HAS_THIRDPARTY=1
        ;;
    q|Q|*)
        echo -e "${RED}❌ Install Genesis-X Aborting.${RESET}"
        exit 0
        ;;
esac

if [ ${#FEATURES[@]} -eq 0 ]; then
    echo -e "${YELLOW}⚠️  No features selected, aborting install${RESET}"
    exit 0
fi

echo "Installing features: ${FEATURES[*]}"
echo "Target project: $TARGET"
echo

install_snippets() {
    local SRC_SNIPPETS_FILE="$(dirname "$0")/../tools/qtcreator-snippets/snippets.xml"

    if [ ! -f "$SRC_SNIPPETS_FILE" ]; then
        echo "❌ snippets file not found:"
        echo "   $SRC_SNIPPETS_FILE"
        echo "   (adjust the path in snippets)"
        return 1
    fi

    local DEST_DIR=""

    if [ -n "$APPDATA" ]; then
        DEST_DIR="$APPDATA/QtProject/qtcreator/snippets"
    elif [ -d "/mnt/c" ]; then
        local WINUSER
        WINUSER="$(cmd.exe /c "echo %USERNAME%" 2>/dev/null | tr -d '\r')"

        if [ -z "$WINUSER" ]; then
            read -rp "   Windows username for Qt Creator (e.g. Bas): " WINUSER
        fi

        DEST_DIR="/mnt/c/Users/${WINUSER}/AppData/Roaming/QtProject/qtcreator/snippets"
    else
        DEST_DIR="$HOME/.config/QtProject/qtcreator/snippets"
    fi

    echo "   Destination folder:"
    echo "   $DEST_DIR"
    echo

    mkdir -p "$DEST_DIR" || {
        echo "❌ Could not create directory: $DEST_DIR"
        return 1
    }

    local DEST_FILE="$DEST_DIR/snippets.xml"

    if [ ! -f "$DEST_FILE" ]; then
        cp "$SRC_SNIPPETS_FILE" "$DEST_FILE" || {
            echo "❌ Failed to copy snippets to: $DEST_FILE"
            return 1
        }

        echo "✅ Snippets installed:"
        echo "   $DEST_FILE"
        echo
    else
        local BACKUP_FILE="${DEST_FILE}.bak.$(date +%Y%m%d-%H%M%S)"
        cp "$DEST_FILE" "$BACKUP_FILE" || {
            echo "❌ Failed to create backup of existing snippets:"
            echo "   $DEST_FILE"
            return 1
        }
        echo "   Existing snippets.xml found."
        echo "   Backup created:"
        echo "   $BACKUP_FILE"

        local CLEAN_FILE
        CLEAN_FILE="$(mktemp)"

        sed 's|<snippets\s*/>|<snippets>\n<snippets>|' "$DEST_FILE" \
            | grep -v 'trigger="genesisx' \
            > "$CLEAN_FILE"

        local TMP_FILE
        TMP_FILE="$(mktemp)"

        {
            if head -n1 "$CLEAN_FILE" | grep -q '^<\?xml'; then
                head -n1 "$CLEAN_FILE"
            else
                echo '<?xml version="1.0" encoding="UTF-8"?>'
            fi

            echo '<snippets>'

            sed '1,2d;$d' "$CLEAN_FILE"

            sed '1,2d;$d' "$SRC_SNIPPETS_FILE"

            echo '</snippets>'
        } > "$TMP_FILE"

        mv "$TMP_FILE" "$DEST_FILE" || {
            echo "❌ Failed to write merged snippets file."
            echo "   Original is still available as:"
            echo "   $BACKUP_FILE"
            rm -f "$CLEAN_FILE"
            return 1
        }

        rm -f "$CLEAN_FILE"

        echo "✅ Update Genesis-X snippets into existing file:"
        echo "   $DEST_FILE"
        echo "   (Backup: $BACKUP_FILE)"
        echo
    fi
}

register_docs() {
    local QT_CREATOR_INI=""

    if [ -n "$APPDATA" ]; then
        QT_CREATOR_INI="$APPDATA/QtProject/QtCreator.ini"
    elif [ -d "/mnt/c" ]; then
        local WINUSER
        WINUSER="$(cmd.exe /c "echo %USERNAME%" 2>/dev/null | tr -d '\r')"

        if [ -z "$WINUSER" ]; then
            read -rp "   Windows username for Qt Creator (e.g. Bas): " WINUSER
        fi

        QT_CREATOR_INI="/mnt/c/Users/${WINUSER}/AppData/Roaming/QtProject/QtCreator.ini"
    else
        QT_CREATOR_INI="$HOME/.config/QtProject/QtCreator.ini"
        echo "also macos"
    fi

    if [[ ! -f "$QT_CREATOR_INI" ]]; then
        echo "QtCreator.ini not found at: $QT_CREATOR_INI"
        echo "You need to register Genesis-X docs in Qt yourself"
    else
        local SCRIPT_DIR
        SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
        local SRC_DOCS_QCH="$SCRIPT_DIR/../docs/out/GenesisX.qch"
#        local SRC_DOCS_QCH="$(dirname "$0")/../docs/out/GenesisX.qch"
        if ! grep -q "InstalledDocumentation=$SRC_DOCS_QCH" "$QT_CREATOR_INI"; then
            echo "Registering Genesis-X docs in Qt Creator:"
            echo "  INI: $QT_CREATOR_INI"
            echo "  QCH: $SRC_DOCS_QCH"

            tmp="$(mktemp)"

            awk -v qch="$SRC_DOCS_QCH" '
                BEGIN {
                    in_help = 0
                    installed_seen = 0
                }
                /^\[Help\]/ {
                    in_help = 1
                }
                /^\[/ && $0 !~ /^\[Help\]/ {
                    in_help = 0
                }
                {
                    if (in_help && /^InstalledDocumentation=/) {
                        installed_seen = 1
                        # Append our qch if not already present
                        if (index($0, qch) == 0) {
                            $0 = $0 "," qch
                        }
                    }
                    print
                }
                END {
                    if (in_help && !installed_seen) {
                        print "InstalledDocumentation=" qch
                    }
                    if (!in_help && !installed_seen) {
                        print ""
                        print "[Help]"
                        print "InstalledDocumentation=" qch
                    }
                }
            ' "$QT_CREATOR_INI" > "$tmp" && mv "$tmp" "$QT_CREATOR_INI"
        fi
    fi
}


for F in "${FEATURES[@]}"; do
    case "$F" in
        docs)
            echo "→ Building Genesis-X documentation..."
            (
                cd "docs"
                "$QMAKE_CMD" docs.pro
                "$MAKE_CMD" help
            )
            echo "✅ Documentation build finished."
#            cd "../"
            ;;
        snippets)
            echo "→ Installing Qt Creator snippets for Genesis-X..."

            install_snippets
            ;;
        wizard)
            echo "→ Installing Qt Creator wizards for Genesis-X..."
            bash "$(dirname "$0")/../tools/qtcreator-wizard/install-wizard.sh"
            echo "✅ Qt Creator wizard for Genesis-X installed."
            ;;
        thirdparty)
            echo "→ Installing 3rd party for Genesis-X..."
            SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
            bash "$SCRIPT_DIR/../scripts/bootstrap.sh" all
            echo "✅ 3rd party dependencies for Genesis-X installed."
            HAS_THIRDPARTY=1
            ;;
    esac
done

if  [[ $HAS_DOCS -eq 1 ]]; then
    register_docs
fi
#if  [[ $HAS_SNIPPET -eq 1 ]]; then
##    echo "I have snippets"
#fi
#if  [[ $HAS_WIZARD -eq 1 ]]; then
##    echo "I have wizard"
#fi
if  [[ $HAS_THIRDPARTY -eq 1 ]]; then
    echo "====================================="
    echo -e "${YELLOW}⚠️  Manual Step Required${RESET}"
    echo "====================================="
    echo
    echo "Open project Genesis-X in Qt"
    echo -e "Go to: \033[1mcore - Other files - ../3rdparty/firebase_cpp_sdk/Andoird\033[0m"
    echo -e "Open the file: \033[1mfirebase_dependencies.gradle\033[0m"
    echo -e "Go to line: \033[1m108\033[0m"
    echo -e "Encapsule \033[1mproject.extensions.create('firebaseCpp', FirebaseCppExtension)\033[0m e.g."
    echo -e "\033[32mif (!project.extensions.findByName('firebaseCpp')) {\033[0m"
    echo -e "  \033[1mproject.extensions.create('firebaseCpp', FirebaseCppExtension)\033[0m"
    echo -e "\033[32m}\033[0m"
    echo -e "Then go to line: \033[1m113\033[0m"
    echo -e "Change this: \033[31m\${gradle.firebase_cpp_sdk_dir}/libs/android/\${lib}.pro\033[0m"
    echo -e "To this: \033[32m\${firebase_cpp_sdk_dir}/libs/android/\${lib}.pro\033[0m"
    echo -e "And on line: \033[1m134\033[0m"
    echo -e "Change this: \033[31mdirs gradle.firebase_cpp_sdk_dir + \"/libs/android\"\033[0m"
    echo -e "To this: \033[32mdirs firebase_cpp_sdk_dir + \"/libs/android\"\033[0m"
fi

echo
echo -e "${GREEN}✅ Genesis-X installation complete.${RESET}"

WIN_OR_ELSE=0
QTC="${QT_CREATOR_EXE:-}"

if [ -n "$APPDATA" ]; then
    if [[ ! -x "$QTC" ]]; then
        QTC="C:\Qt\Tools\QtCreator\bin\qtcreator.exe"
    fi
    if [[ ! -x "$QTC" ]]; then
        QTC="C:\Program Files\Qt Creator\bin\qtcreator.exe"
    fi
    if [[ ! -x "$QTC" ]]; then
        QTC="C:\Program Files (x86)\Qt Creator\bin\qtcreator.exe"
    fi
    if [[ ! -x "$QTC" ]]; then
        QTC="D:\Qt\Tools\QtCreator\bin\qtcreator.exe"
    fi
    if [[ ! -x "$QTC" ]]; then
        QTC="D:\Program Files\Qt Creator\bin\qtcreator.exe"
    fi
    if [[ ! -x "$QTC" ]]; then
        QTC="D:\Program Files (x86)\Qt Creator\bin\qtcreator.exe"
    fi
    if [[ ! -x "$QTC" ]]; then
        QTC="E:\Qt\Tools\QtCreator\bin\qtcreator.exe"
    fi
    if [[ ! -x "$QTC" ]]; then
        QTC="E:\Program Files\Qt Creator\bin\qtcreator.exe"
    fi
    if [[ ! -x "$QTC" ]]; then
        QTC="E:\Program Files (x86)\Qt Creator\bin\qtcreator.exe"
    fi
elif [ -d "/mnt/c" ]; then
    if [[ ! -x "$QTC" ]]; then
        QTC="/mnt/c/Qt/Tools/QtCreator/bin/qtcreator.exe"
    fi
    if [[ ! -x "$QTC" ]]; then
        QTC="/mnt/c/Program Files/Qt Creator/bin/qtcreator.exe"
    fi
    if [[ ! -x "$QTC" ]]; then
        QTC="/mnt/c/Program Files (x86)/Qt Creator/bin/qtcreator.exe"
    fi
    if [[ ! -x "$QTC" ]]; then
        QTC="/mnt/d/Qt/Tools/QtCreator/bin/qtcreator.exe"
    fi
    if [[ ! -x "$QTC" ]]; then
        QTC="/mnt/d/Program Files/Qt Creator/bin/qtcreator.exe"
    fi
    if [[ ! -x "$QTC" ]]; then
        QTC="/mnt/d/Program Files (x86)/Qt Creator/bin/qtcreator.exe"
    fi
    if [[ ! -x "$QTC" ]]; then
        QTC="/mnt/e/Qt/Tools/QtCreator/bin/qtcreator.exe"
    fi
    if [[ ! -x "$QTC" ]]; then
        QTC="/mnt/e/Program Files/Qt Creator/bin/qtcreator.exe"
    fi
    if [[ ! -x "$QTC" ]]; then
        QTC="/mnt/e/Program Files (x86)/Qt Creator/bin/qtcreator.exe"
    fi
else
    if [[ ! -x "$QTC" ]]; then
        QTC="/opt/Qt/Tools/QtCreator/bin/qtcreator"      # adjust to your real path
    elif [[ ! -x "$QTC" ]]; then
        QTC="/usr/bin/qtcreator"
    elif [[ ! -x "$QTC" ]]; then
        QTC="/usr/local/bin/qtcreator"
    elif [[ ! -x "$QTC" ]]; then
        QTC="$HOME/Qt/Qt Creator.app"
    fi
fi
echo "$QTC"
if [[ ! -x "$QTC" ]]; then
    QT_CREATOR_BIN="${QT_CREATOR_BIN:-}"

    if [ -z "$QT_CREATOR_BIN" ]; then
        # 1) If qtcreator is in PATH, prefer that
        if command -v qtcreator >/dev/null 2>&1; then
            QT_CREATOR_BIN="$(command -v qtcreator)"
        # 2) Fallback to the Qt Online Installer location you found
        elif [ -x "$HOME/Qt/Tools/QtCreator/bin/qtcreator" ]; then
            QT_CREATOR_BIN="$HOME/Qt/Tools/QtCreator/bin/qtcreator"
        elif [ -x "$HOME/Qt/Qt Creator.app" ]; then
            QT_CREATOR_BIN="$HOME/Qt/Qt Creator.app/Contents/MacOS/Qt Creator"
        fi
    fi
    if [ -n "$QT_CREATOR_BIN" ]; then
        echo "Launching Qt Creator: $QT_CREATOR_BIN"
        "$QT_CREATOR_BIN" &
        exit 0
    else
        echo -e "${YELLOW}⚠We could not detect a working Qt version, start Qt manually${RESET}"
        exit 0
    fi
else
    echo -e "${GREEN}✅ Working Qt version found, starting Qt now to let changed setting take effect${RESET}"
    "$QTC"
    exit 0
fi
