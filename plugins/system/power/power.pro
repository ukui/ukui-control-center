QT       += widgets

TARGET = $$qtLibraryTarget(power)
TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

DESTDIR = ../..

LIBS += -L/usr/lib/ -lgsettings-qt


FORMS += \
    power.ui

HEADERS += \
    power.h

SOURCES += \
    power.cpp
