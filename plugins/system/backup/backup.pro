#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T09:55:08
#
#-------------------------------------------------
include(../../../env.pri)

QT       += widgets svg dbus

TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(backup)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}
INSTALLS += target

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \

##加载gio库和gio-unix库
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0



#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        backup.cpp \
        messagebox.cpp \
        messageboxdialog.cpp \
        messageboxpower.cpp

HEADERS += \
        backup.h \
        messagebox.h \
        messageboxdialog.h \
        messageboxpower.h

FORMS += \
        backup.ui \
        messagebox.ui \
        messageboxdialog.ui
