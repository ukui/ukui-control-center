QT       += widgets dbus

TARGET = $$qtLibraryTarget(about)
TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

DESTDIR = ../..

#include(../../component/component.pri)

#DEFINES += QT_DEPRECATED_WARNINGS


HEADERS += \
    about.h

SOURCES += \
    about.cpp

FORMS += \
    about.ui

RESOURCES += \
    res/img.qrc
