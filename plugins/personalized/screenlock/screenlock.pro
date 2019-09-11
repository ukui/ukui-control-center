#-------------------------------------------------
#
# Project created by QtCreator 2019-06-26T08:25:40
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

TARGET = $$qtLibraryTarget(screenlock)
DESTDIR = ../..

include(../../pluginsComponent/pluginsComponent.pri)

##加载gio库和gio-unix库
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0

#DEFINES += QT_DEPRECATED_WARNINGS

target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
INSTALLS += target


SOURCES += \
        screenlock.cpp

HEADERS += \
        screenlock.h

FORMS += \
        screenlock.ui
