#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T13:53:10
#
#-------------------------------------------------

include(../../../env.pri)

QT       += widgets

TEMPLATE = lib
CONFIG += plugin \
          += c++11 \
          link_pkgconfig

PKGCONFIG     += gsettings-qt

TARGET = $$qtLibraryTarget(vpn)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \

LIBS += -lpolkit-qt5-core-1
#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    vpn.cpp

HEADERS += \
    vpn.h

FORMS += \
    vpn.ui

INSTALLS += target
