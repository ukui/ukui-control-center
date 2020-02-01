#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T13:53:10
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(vpn)
DESTDIR = ../../../pluginlibs

include(../../../env.pri)

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \


#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        vpn.cpp

HEADERS += \
        vpn.h

FORMS += \
        vpn.ui
