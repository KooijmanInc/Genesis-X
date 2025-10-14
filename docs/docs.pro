TEMPLATE = aux

QMAKE_EXTRA_TARGETS += docs arrange help

QMAKE_DOCS = $$PWD/genesisx.qdocconf

QMAKE_DOCS_OUTPUTDIR = $$OUT_PWD/out
QDOC        = $$[QT_INSTALL_BINS]/qdoc
QHELPGEN    = $$[QT_INSTALL_BINS]/qhelpgenerator

# message(QMAKE_DOCS: $$QMAKE_DOCS and QMAKE_DOCS_OUTPUTDIR: $$QMAKE_DOCS_OUTPUTDIR)

docs.commands = rmdir /S /Q "$$shell_path($$QMAKE_DOCS_OUTPUTDIR)" 2>nul & mkdir "$$shell_path($$QMAKE_DOCS_OUTPUTDIR)" & \
    "\"$$QDOC\"" "\"$$shell_path($$OUT_PWD/genesisx.qdocconf)\""

arrange.depends = docs

help.depends = arrange
help.commands = "\"$$shell_path($$QHELPGEN)\"" "\"$$shell_path($$QMAKE_DOCS_OUTPUTDIR/GenesisX.qhp)\"" -o "\"$$shell_path($$QMAKE_DOCS_OUTPUTDIR/GenesisX.qch)\""
