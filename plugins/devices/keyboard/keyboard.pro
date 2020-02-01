#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------

QT       += widgets x11extras
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(keyboard)
DESTDIR = ../../../pluginlibs

include(../../../env.pri)

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \
                 /usr/include/ukuisdk \

LIBS          +=  -L/usr/lib/ -lkeyboardclient -linterfaceclient -lgsettings-qt

CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += libmatekbd \

#inst1.files += org.ukui.control-center.keybinding.gschema.xml
#inst1.path = /usr/share/glib-2.0/schemas
#target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
#INSTALLS += target \
#            inst1


#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        keyboardcontrol.cpp \
    kbdlayoutmanager.cpp

HEADERS += \
        keyboardcontrol.h \
    kbdlayoutmanager.h

FORMS += \
        keyboardcontrol.ui \
    kbdlayoutmanager.ui
