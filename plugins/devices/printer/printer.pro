#-------------------------------------------------
#
# Project created by QtCreator 2019-02-28T14:09:42
#
#-------------------------------------------------

QT       += widgets printsupport

TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(printer)
DESTDIR = ../../../pluginlibs

include(../../../env.pri)

INCLUDEPATH += \
               $$PROJECT_ROOTDIR \


#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        printer.cpp

HEADERS += \
        printer.h

FORMS += \
        printer.ui
