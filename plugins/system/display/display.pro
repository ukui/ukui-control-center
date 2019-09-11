#-------------------------------------------------
#
# Project created by QtCreator 2019-02-20T15:36:43
#
#-------------------------------------------------

TEMPLATE = lib
CONFIG        += plugin
QT            += widgets
INCLUDEPATH   += ../../.. \
                 /usr/include/mate-desktop-2.0/libmate-desktop

#LIBS += -L/usr/lib/ -lmate-desktop-2

TARGET = $$qtLibraryTarget(display)
DESTDIR = ../..

include(../../pluginsComponent/pluginsComponent.pri)

##加载gio库和gio-unix库
CONFIG        += link_pkgconfig \
                 C++11

PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \
                 mate-desktop-2.0 \
                 dbus-glib-1

target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
INSTALLS += target

SOURCES += \
        display.cpp

HEADERS += \
        display.h

FORMS += \
    display.ui
