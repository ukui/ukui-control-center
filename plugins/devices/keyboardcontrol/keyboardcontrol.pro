#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------

QT       += widgets x11extras

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = $$qtLibraryTarget(keyboardcontrol)
TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../.. \
                /usr/include/kylinsdk \

LIBS          +=  -L/usr/lib/ -lkeyboardclient -linterfaceclient -lgsettings-qt

CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \
                 libmatekbd

DESTDIR = ../..

include(../../component/component.pri)

target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
INSTALLS += target


#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        keyboardcontrol.cpp \
#    keyboardlayout.cpp \
    kbdlayoutmanager.cpp \
    customlineedit.cpp

HEADERS += \
        keyboardcontrol.h \
#    keyboardlayout.h \
    kbdlayoutmanager.h \
    customlineedit.h

FORMS += \
        keyboardcontrol.ui \
    kbdlayoutmanager.ui
