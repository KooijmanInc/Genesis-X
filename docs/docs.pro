# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

QT += core gui qml quick

TEMPLATE = aux

QMAKE_EXTRA_TARGETS += docs arrange help

# QMAKE_DOCS = $$PWD/genesisx.qdocconf

# QDOCCONF_IN  = $$PWD/docs/qt-includes.qdocconf.in

# QMAKE_SUBSTITUTES += $$QDOCCONF_IN

# QDOCCONF_OUT = $$PWD/docs/qt-includes.qdocconf

# # Vars to substitute (come from the user’s Qt installation)
# QT_HEADERS     = $$[QT_INSTALL_HEADERS]
# QMAKE_MKSPECS  = $$[QMAKE_MKSPECS]



# # Ensure the output lands where your main qdocconf can include it
# QMAKE_SUBSTITUTES_OUTPUT = $$QDOCCONF_OUT

QMAKE_DOCS_OUTPUTDIR = $$OUT_PWD/out
QDOC        = $$[QT_INSTALL_BINS]/qdoc
QHELPGEN    = $$[QT_INSTALL_BINS]/qhelpgenerator

# message(QMAKE_DOCS: $$QMAKE_DOCS and QMAKE_DOCS_OUTPUTDIR: $$QMAKE_DOCS_OUTPUTDIR)

docs.commands = rmdir /S /Q "$$shell_path($$QMAKE_DOCS_OUTPUTDIR)" 2>nul & mkdir "$$shell_path($$QMAKE_DOCS_OUTPUTDIR)" & \
    "\"$$QDOC\"" "\"$$shell_path($$OUT_PWD/genesisx.qdocconf)\""

arrange.depends = docs

help.depends = arrange
help.commands = "\"$$shell_path($$QHELPGEN)\"" "\"$$shell_path($$QMAKE_DOCS_OUTPUTDIR/GenesisX.qhp)\"" -o "\"$$shell_path($$QMAKE_DOCS_OUTPUTDIR/GenesisX.qch)\""
