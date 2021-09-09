#-------------------------------------------------
#
# Project created by QtCreator 2021-07-20T13:54:31
#
#-------------------------------------------------

include(../../../env.pri)
include($$PROJECT_COMPONENTSOURCE/switchbutton.pri)
include($$PROJECT_COMPONENTSOURCE/hoverbtn.pri)
include($$PROJECT_COMPONENTSOURCE/label.pri)
include($$PROJECT_COMPONENTSOURCE/infobutton.pri)


QT       += widgets network dbus gui core
TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(wlanconnect)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \

LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt

CONFIG += c++11 \
          link_pkgconfig \

PKGCONFIG += gsettings-qt \

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    deviceframe.cpp \
    drownlabel.cpp \
    itemframe.cpp \
    wlanconnect.cpp \
    wlanitem.cpp

HEADERS += \
    deviceframe.h \
    drownlabel.h \
    itemframe.h \
    wlanconnect.h \
    wlanitem.h

FORMS += \
    wlanconnect.ui

INSTALLS += target
