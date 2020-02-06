#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T13:45:31
#
#-------------------------------------------------

QT       += widgets network
TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(netconnect)
DESTDIR = ../../../pluginlibs

include(../../../env.pri)

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        netconnect.cpp

HEADERS += \
        netconnect.h

FORMS += \
        netconnect.ui
