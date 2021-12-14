#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T15:14:42
#
#-------------------------------------------------

QT       += widgets dbus KWidgetsAddons

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

include(../../../env.pri)

TARGET = $$qtLibraryTarget(area)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}
INSTALLS += target

LIBS += -L$$[QT_INSTALL_DIRS] -lgsettings-qt

##加载gio库和gio-unix库，用于处理时间
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \
                 gsettings-qt \

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        area.cpp \
        languageFrame.cpp

HEADERS += \
        area.h \
        languageFrame.h

FORMS += \
        area.ui
