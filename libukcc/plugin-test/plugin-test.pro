QT += widgets

TEMPLATE = lib
CONFIG += plugin \
          link_pkgconfig

LIBS += -lukcc

TARGET = $$qtLibraryTarget(plugin-test)
target.path = $$[QT_INSTALL_LIBS]/ukui-control-center


TRANSLATIONS += translations/zh_CN.ts

isEmpty(PREFIX) {
    PREFIX = /usr
}

qm_files.path = $${PREFIX}/share/plugin-test/translations
qm_files.files = translations/*

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}


INSTALLS += target \
            qm_files \

HEADERS += \
    ukccplugin.h

SOURCES += \
    ukccplugin.cpp
