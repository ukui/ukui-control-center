QT       += widgets
TEMPLATE = lib
CONFIG   += plugin

TARGET = $$qtLibraryTarget(power)
DESTDIR = ../../../pluginlibs

include(../../../env.pri)

INCLUDEPATH += \
               $$PROJECT_ROOTDIR \


LIBS     += -L/usr/lib/ -lgsettings-qt

target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
INSTALLS += target

FORMS += \
    power.ui

HEADERS += \
    power.h \
    powermacrodata.h

SOURCES += \
    power.cpp
