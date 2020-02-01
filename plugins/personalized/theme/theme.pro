#-------------------------------------------------
#
# Project created by QtCreator 2019-06-21T10:50:19
#
#-------------------------------------------------
QT       += widgets

TEMPLATE = lib
CONFIG += plugin


TARGET = $$qtLibraryTarget(theme)
DESTDIR = ../../../pluginlibs


include(../../../env.pri)
include($$PROJECT_COMPONENTSOURCE/switchbutton.pri)

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \

LIBS          += -L/usr/lib/ -lgsettings-qt

#DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
        theme.cpp \
    themewidget.cpp

HEADERS += \
        theme.h \
    themewidget.h

FORMS += \
        theme.ui

RESOURCES +=
