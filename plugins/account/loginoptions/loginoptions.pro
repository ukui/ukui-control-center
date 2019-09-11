#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T14:44:56
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

TARGET = $$qtLibraryTarget(loginoptions)
DESTDIR = ../..

include(../../pluginsComponent/pluginsComponent.pri)

#DEFINES += QT_DEPRECATED_WARNINGS

target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
INSTALLS += target

SOURCES += \
        loginoptions.cpp

HEADERS += \
        loginoptions.h

FORMS += \
        loginoptions.ui
