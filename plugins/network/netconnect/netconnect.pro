#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T13:45:31
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

TARGET = $$qtLibraryTarget(netconnect)
DESTDIR = ../..

#DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
        netconnect.cpp

HEADERS += \
        netconnect.h

FORMS += \
        netconnect.ui
