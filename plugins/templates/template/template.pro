#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------

QT       += widgets

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = $$qtLibraryTarget(template)
TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

DESTDIR = ../..

include(../../pluginsComponent/pluginsComponent.pri)

target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
INSTALLS += target


#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        template.cpp

HEADERS += \
        template.h

FORMS += \
        template.ui
