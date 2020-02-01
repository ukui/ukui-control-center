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

include(../../pluginsComponent/pluginsComponent.pri)

#DEFINES += QT_DEPRECATED_WARNINGS

target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
INSTALLS += target


SOURCES += \
        recovery.cpp

HEADERS += \
        recovery.h

FORMS += \
        recovery.ui
