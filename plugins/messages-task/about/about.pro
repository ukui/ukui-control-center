QT       += widgets

TARGET = $$qtLibraryTarget(about)
TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

DESTDIR = ../..

#DEFINES += QT_DEPRECATED_WARNINGS


FORMS += \
    about.ui

HEADERS += \
    about.h

SOURCES += \
    about.cpp

RESOURCES += \
    res/img.qrc
