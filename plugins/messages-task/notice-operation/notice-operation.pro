#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T15:31:50
#
#-------------------------------------------------

QT       += widgets

TARGET = $$qtLibraryTarget(notice-operation)
TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

DESTDIR = ../..

include(../../pluginsComponent/pluginsComponent.pri)

#DEFINES += QT_DEPRECATED_WARNINGS

target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
INSTALLS += target

SOURCES += \
        noticeoperation.cpp

HEADERS += \
        noticeoperation.h

FORMS += \
        noticeoperation.ui
