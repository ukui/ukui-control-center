#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T14:35:43
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

TARGET = $$qtLibraryTarget(userinfo)
DESTDIR = ../..

#DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
        userinfo.cpp

HEADERS += \
        userinfo.h

FORMS += \
        userinfo.ui
