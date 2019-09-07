#-------------------------------------------------
#
# Project created by QtCreator 2019-06-17T11:16:02
#
#-------------------------------------------------

QT       += widgets xml

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

TARGET = $$qtLibraryTarget(wallpaper)
DESTDIR = ../..

include(../../component/component.pri)

LIBS += -L/usr/lib/ -lgsettings-qt

#DEFINES += QT_DEPRECATED_WARNINGS

target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
INSTALLS += target

SOURCES += \
        wallpaper.cpp \
    xmlhandle.cpp \
    component/custdomitemmodel.cpp

HEADERS += \
        wallpaper.h \
    xmlhandle.h \
    component/custdomitemmodel.h

FORMS += \
        wallpaper.ui
