#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T11:47:23
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

TARGET = $$qtLibraryTarget(update)
DESTDIR = ../..

include(../../pluginsComponent/pluginsComponent.pri)

#DEFINES += QT_DEPRECATED_WARNINGS

target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
INSTALLS += target

SOURCES += \
        update.cpp

HEADERS += \
        update.h

FORMS += \
        update.ui
