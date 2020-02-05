#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T15:22:24
#
#-------------------------------------------------

QT       += widgets

TARGET = $$qtLibraryTarget(multitask)
TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

DESTDIR = ../..

include(../../pluginsComponent/pluginsComponent.pri)

#DEFINES += QT_DEPRECATED_WARNINGS

target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
INSTALLS += target

SOURCES += \
        multitask.cpp

HEADERS += \
        multitask.h

FORMS += \
        multitask.ui
