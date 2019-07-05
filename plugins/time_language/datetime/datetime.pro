#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T15:08:41
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

TARGET = $$qtLibraryTarget(datetime)
DESTDIR = ../..

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        datetime.cpp

HEADERS += \
        datetime.h

FORMS += \
        datetime.ui
