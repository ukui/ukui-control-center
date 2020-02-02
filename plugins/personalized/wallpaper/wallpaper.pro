#-------------------------------------------------
#
# Project created by QtCreator 2019-06-17T11:16:02
#
#-------------------------------------------------

QT       += widgets xml

TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(wallpaper)
DESTDIR = ../../../pluginlibs

include(../../../env.pri)

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \

LIBS += -L/usr/lib/ -lgsettings-qt

##加载gio库和gio-unix库，用于获取和设置enum类型的gsettings
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        wallpaper.cpp \
    xmlhandle.cpp \
    component/custdomitemmodel.cpp \
    simplethread.cpp \
    workerobject.cpp

HEADERS += \
        wallpaper.h \
    xmlhandle.h \
    component/custdomitemmodel.h \
    simplethread.h \
    workerobject.h

FORMS += \
        wallpaper.ui
