# SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
# Copyright (c) 2025 Kooijman Incorporate Holding B.V.

QT += core gui qml quick

TEMPLATE = aux

QMAKE_EXTRA_TARGETS += docs arrange help

isEmpty(QMAKE_DOCS_OUTPUTDIR) {
    QMAKE_DOCS_OUTPUTDIR = $$OUT_PWD/out
}
isEmpty(QDOC) {
    QDOC        = $$[QT_INSTALL_BINS]/qdoc
}
isEmpty(QHELPGEN) {
    QHELPGEN    = $$[QT_INSTALL_BINS]/qhelpgenerator
}

# message(QMAKE_DOCS: $$QMAKE_DOCS and QMAKE_DOCS_OUTPUTDIR: $$QMAKE_DOCS_OUTPUTDIR)

win32 {
    docs.commands = rmdir /S /Q "$$shell_path($$QMAKE_DOCS_OUTPUTDIR)" 2>nul & mkdir "$$shell_path($$QMAKE_DOCS_OUTPUTDIR)" & \
        "\"$$QDOC\"" "\"$$shell_path($$OUT_PWD/genesisx.qdocconf)\""

    arrange.depends = docs

    help.depends = arrange
    help.commands = "\"$$shell_path($$QHELPGEN)\"" "\"$$shell_path($$QMAKE_DOCS_OUTPUTDIR/GenesisX.qhp)\"" -o "\"$$shell_path($$QMAKE_DOCS_OUTPUTDIR/GenesisX.qch)\""
} else {
    docs.commands = rm -rf "$$QMAKE_DOCS_OUTPUTDIR" && \
                    mkdir -p "$$QMAKE_DOCS_OUTPUTDIR" && \
                    "$$QDOC" "$$OUT_PWD/genesisx.qdocconf"

    arrange.depends = docs

    help.depends = arrange
    help.commands = "$$QHELPGEN" \
                    "$$QMAKE_DOCS_OUTPUTDIR/GenesisX.qhp" \
                    -o "$$QMAKE_DOCS_OUTPUTDIR/GenesisX.qch"
}
