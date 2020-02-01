#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T09:55:08
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(backup)
DESTDIR = ../../../pluginlibs

include(../../../env.pri)

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \



#DEFINES += QT_DEPRECATED_WARNINGS

target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
INSTALLS += target

SOURCES += \
        backup.cpp

HEADERS += \
        backup.h

FORMS += \
        backup.ui
