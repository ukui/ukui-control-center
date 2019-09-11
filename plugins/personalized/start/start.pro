#-------------------------------------------------
#
# Project created by QtCreator 2019-06-26T10:59:46
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

TARGET = $$qtLibraryTarget(start)
DESTDIR = ../..

include(../../pluginsComponent/pluginsComponent.pri)

LIBS += -L/usr/lib/ -lgsettings-qt

#DEFINES += QT_DEPRECATED_WARNINGS

target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
INSTALLS += target

SOURCES += \
        start.cpp

HEADERS += \
        start.h

FORMS += \
        start.ui
