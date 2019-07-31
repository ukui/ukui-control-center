#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T11:56:39
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

TARGET = $$qtLibraryTarget(recovery)
DESTDIR = ../..

#DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
        recovery.cpp

HEADERS += \
        recovery.h

FORMS += \
        recovery.ui
