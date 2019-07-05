#-------------------------------------------------
#
# Project created by QtCreator 2019-06-25T12:51:26
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

TARGET = $$qtLibraryTarget(fonts)
DESTDIR = ../..

##加载gio库和gio-unix库，用于获取gsettings的默认值
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0

LIBS += -L/usr/lib/ -lgsettings-qt

#DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
        fonts.cpp

HEADERS += \
        fonts.h

FORMS += \
        fonts.ui
