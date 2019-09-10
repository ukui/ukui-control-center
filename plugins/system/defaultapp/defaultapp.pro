#-------------------------------------------------
#
# Project created by QtCreator 2019-02-20T15:36:43
#
#-------------------------------------------------

TEMPLATE = lib
CONFIG        += plugin
QT            += widgets
##加载gio库和gio-unix库，用于处理desktop文件
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0


INCLUDEPATH   += ../../.. \
                 /usr/include/kylinsdk

LIBS          += -L/usr/lib/ -ldefaultprograms

TARGET = $$qtLibraryTarget(defaultapp)
DESTDIR = ../..

include(../../component/component.pri)

target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
INSTALLS += target

SOURCES += \
        defaultapp.cpp \
    addappdialog.cpp \
    component/custdomcombobox.cpp

HEADERS += \
        defaultapp.h \
    addappdialog.h \
    component/custdomcombobox.h

FORMS += \
    defaultapp.ui \
    addappdialog.ui
