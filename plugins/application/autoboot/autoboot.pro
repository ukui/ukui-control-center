#-------------------------------------------------
#
# Project created by QtCreator 2019-03-06T14:45:57
#
#-------------------------------------------------
include(../../../env.pri)

QT       += widgets svg dbus
TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(autoboot)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}
INSTALLS += target

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \

##加载gio库和gio-unix库，用于处理desktop文件
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \
                 gsettings-qt

#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    autoboot.cpp \
    addautoboot.cpp \
    rmenu.cpp

HEADERS += \
    autoboot.h \
    addautoboot.h \
    datadefined.h \
    rmenu.h

FORMS +=

RESOURCES += \
    res/img.qrc

DISTFILES += \
    addbtn \
    addbtn.pri
