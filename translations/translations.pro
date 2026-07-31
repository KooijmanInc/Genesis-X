TEMPLATE = aux

SOURCES += $$files($$PWD/../assets/*.qml, true)

TRANSLATIONS = \
    i18n/genesisx_de.ts \
    i18n/genesisx_en.ts \
    i18n/genesisx_es.ts \
    i18n/genesisx_fr.ts \
    i18n/genesisx_nl.ts

RESOURCES += \
    translations.qrc \
    translations_source.qrc