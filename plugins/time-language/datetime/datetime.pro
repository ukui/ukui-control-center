#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T15:08:41
#
#-------------------------------------------------

QT       += widgets dbus

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

TARGET = $$qtLibraryTarget(datetime)
DESTDIR = ../..

include(../../component/component.pri)

LIBS += -L/usr/lib/ -lgsettings-qt

##加载gio库和gio-unix库，用于处理时间
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        datetime.cpp

HEADERS += \
        datetime.h

FORMS += \
        datetime.ui

RESOURCES += \
    tz.qrc
