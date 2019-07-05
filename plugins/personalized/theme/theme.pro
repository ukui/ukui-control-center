#-------------------------------------------------
#
# Project created by QtCreator 2019-06-21T10:50:19
#
#-------------------------------------------------
QT       += widgets

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

TARGET = $$qtLibraryTarget(theme)
DESTDIR = ../..

LIBS += -L/usr/lib/ -lgsettings-qt

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        theme.cpp

HEADERS += \
        theme.h

FORMS += \
        theme.ui

RESOURCES += \
    res/img.qrc
