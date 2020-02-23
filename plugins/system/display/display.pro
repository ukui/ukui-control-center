#-------------------------------------------------
#
# Project created by QtCreator 2019-02-20T15:36:43
#
#-------------------------------------------------
QT            += widgets core gui quickwidgets quick xml  KConfigCore KConfigWidgets KWidgetsAddons
TEMPLATE = lib
CONFIG        += plugin

TARGET = $$qtLibraryTarget(display)
DESTDIR = ../../../pluginlibs

##加载gio库和gio-unix库
CONFIG        += link_pkgconfig \
                 C++11

PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \
                 mate-desktop-2.0 \
                 dbus-glib-1

include(../../../env.pri)
include($$PROJECT_COMPONENTSOURCE/switchbutton.pri)

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \
                 /usr/include/ukuisdk \

#LIBS          += -L$$PROJECT_COMPONENTLIBS -lcommoncomponent
LIBS += -L/usr/lib/ -lgsettings-qt

target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
INSTALLS += target


SOURCES += \
        display.cpp \
    declarative/qmloutput.cpp \
    declarative/qmloutputcomponent.cpp \
    declarative/qmlscreen.cpp \
    controlpanel.cpp \
    outputconfig.cpp \
    resolutionslider.cpp \
    unifiedoutputconfig.cpp \
    utils.cpp \
    widget.cpp

HEADERS += \
        display.h \
    declarative/qmloutput.h \
    declarative/qmloutputcomponent.h \
    declarative/qmlscreen.h \
    controlpanel.h \
    outputconfig.h \
    resolutionslider.h \
    unifiedoutputconfig.h \
    utils.h \
    widget.h

FORMS += \
    display.ui

#DISTFILES += \
#    qml/main.qml \
#    qml/Output.qml

RESOURCES += \
    qml.qrc
