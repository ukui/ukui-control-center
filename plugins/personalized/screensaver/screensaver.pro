#-------------------------------------------------
#
# Project created by QtCreator 2019-06-25T13:24:04
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

TARGET = $$qtLibraryTarget(screensaver)
DESTDIR = ../..

LIBS += -L/usr/lib/ -lgsettings-qt

##加载gio库和gio-unix库
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0

#DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
        screensaver.cpp

HEADERS += \
        screensaver.h

FORMS += \
        screensaver.ui
