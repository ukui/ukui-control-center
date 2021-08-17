#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T11:47:23
#
#-------------------------------------------------

include(../../../env.pri)
include($$PROJECT_COMPONENTSOURCE/label.pri)

QT       += widgets

TEMPLATE = lib
CONFIG += plugin link_pkgconfig

TARGET = $$qtLibraryTarget(update)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}
INSTALLS += target

LIBS          +=  -L$$[QT_INSTALL_LIBS] -lXi -lgsettings-qt

PKGCONFIG     += gsettings-qt

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    update.cpp

HEADERS += \
    update.h

FORMS += \
    update.ui
