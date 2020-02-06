#-------------------------------------------------
#
# Project created by QtCreator 2019-06-26T10:59:46
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(desktop)
DESTDIR = ../../../pluginlibs

include(../../../env.pri)
include($$PROJECT_COMPONENTSOURCE/switchbutton.pri)

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \

LIBS          += -L/usr/lib/ -lgsettings-qt

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        desktop.cpp

HEADERS += \
        desktop.h

FORMS += \
        desktop.ui
