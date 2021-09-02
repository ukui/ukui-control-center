#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------

include(../../../env.pri)
include($$PROJECT_COMPONENTSOURCE/flowlayout.pri)
include($$PROJECT_COMPONENTSOURCE/label.pri)

QT       += widgets x11extras

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = $$qtLibraryTarget(gesture)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

TEMPLATE = lib
CONFIG += plugin

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \
#LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt

CONFIG += link_pkgconfig c++11

PKGCONFIG += gsettings-qt

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    gesture.cpp \
    itemwidget.cpp

HEADERS += \
    gesture.h \
    itemwidget.h

FORMS += \
    gesture.ui \
    itemwidget.ui

INSTALLS += target
