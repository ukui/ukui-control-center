#-------------------------------------------------
#
# Project created by QtCreator 2019-03-06T14:45:57
#
#-------------------------------------------------

QT       += widgets svg

TARGET = $$qtLibraryTarget(autoboot)
TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../..

##加载gio库和gio-unix库，用于处理desktop文件
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0

DESTDIR = ../..

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        autoboot.cpp \

HEADERS += \
        autoboot.h \

FORMS += \
        autoboot.ui
