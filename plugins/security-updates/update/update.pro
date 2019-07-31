#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T11:47:23
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

TARGET = $$qtLibraryTarget(update)
DESTDIR = ../..

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        update.cpp

HEADERS += \
        update.h

FORMS += \
        update.ui
