#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T15:14:42
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

TARGET = $$qtLibraryTarget(area)
DESTDIR = ../..

#DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
        area.cpp

HEADERS += \
        area.h

FORMS += \
        area.ui
