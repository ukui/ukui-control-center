#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T15:00:57
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

TARGET = $$qtLibraryTarget(language)
DESTDIR = ../..

include(../../component/component.pri)

#DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
        language.cpp

HEADERS += \
        language.h

FORMS += \
        language.ui
