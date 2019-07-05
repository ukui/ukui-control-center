#-------------------------------------------------
#
# Project created by QtCreator 2019-02-20T15:36:43
#
#-------------------------------------------------

TEMPLATE = lib
CONFIG        += plugin
QT            += widgets
INCLUDEPATH   += ../../..

TARGET = $$qtLibraryTarget(display)
DESTDIR = ../..

SOURCES += \
        display.cpp

HEADERS += \
        display.h

FORMS += \
    display.ui
