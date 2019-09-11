QT       += widgets

TARGET = $$qtLibraryTarget(power)
TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

DESTDIR = ../..

LIBS += -L/usr/lib/ -lgsettings-qt

include(../../pluginsComponent/pluginsComponent.pri)

target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
INSTALLS += target

FORMS += \
    power.ui

HEADERS += \
    power.h

SOURCES += \
    power.cpp
