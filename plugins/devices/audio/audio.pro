#-------------------------------------------------
#
# Project created by QtCreator 2019-05-30T09:45:54
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

TARGET = $$qtLibraryTarget(audio)
DESTDIR = ../..

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        audio.cpp

HEADERS += \
        audio.h

FORMS += \
        audio.ui
