QT       += widgets dbus

TARGET = $$qtLibraryTarget(about)
TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

DESTDIR = ../..

#DEFINES += QT_DEPRECATED_WARNINGS


FORMS += \
    about.ui

HEADERS += \
    about.h \
    runroot.h

SOURCES += \
    about.cpp \
    runroot.cpp

RESOURCES += \
    res/img.qrc
