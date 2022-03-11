#-------------------------------------------------
#
# Project created by QtCreator 2019-02-28T14:09:42
#
#-------------------------------------------------

include(../../../env.pri)

QT       += widgets printsupport

LIBS          += -L$$[QT_INSTALL_LIBS] -lcups

TEMPLATE = lib
CONFIG += plugin \
          link_pkgconfig


PKGCONFIG += gsettings-qt
TARGET = $$qtLibraryTarget(printer)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH += \
               $$PROJECT_ROOTDIR \

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        printer.cpp \
        usbthread.cpp

HEADERS += \
        printer.h \
        usbthread.h

FORMS +=

INSTALLS += target
