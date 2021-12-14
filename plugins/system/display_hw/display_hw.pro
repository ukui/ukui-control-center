#-------------------------------------------------
#
# Project created by QtCreator 2019-02-20T15:36:43
#
#-------------------------------------------------
include(../../../env.pri)

QT            += widgets core gui quickwidgets quick xml KScreen  dbus concurrent KConfigCore
TEMPLATE = lib
CONFIG        += c++11   link_pkgconfig plugin

TARGET = $$qtLibraryTarget(display_hw)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}
INSTALLS += target

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \

LIBS          += -L$$[QT_INSTALL_LIBS] -lgsettings-qt

PKGCONFIG += gsettings-qt     \

QMAKE_CXXFLAGS *= -D_FORTIFY_SOURCE=2 -O2

SOURCES += \
    brightnessFrame.cpp \
    declarative/qmloutput.cpp \
    declarative/qmloutputcomponent.cpp \
    declarative/qmlscreen.cpp \
    controlpanel.cpp \
    display_hw.cpp \
    outputconfig.cpp \
    resolutionslider.cpp \
    unifiedoutputconfig.cpp \
    utils.cpp \
    widget.cpp \
    displayperformancedialog.cpp

HEADERS += \
    brightnessFrame.h \
    colorinfo.h \
    declarative/qmloutput.h \
    declarative/qmloutputcomponent.h \
    declarative/qmlscreen.h \
    controlpanel.h \
    display_hw.h \
    outputconfig.h \
    resolutionslider.h \
    scalesize.h \
    unifiedoutputconfig.h \
    utils.h \
    widget.h \
    displayperformancedialog.h

FORMS += \
    display_hw.ui \
    displayperformancedialog.ui

#DISTFILES += \
#    qml/main.qml \
#    qml/Output.qml

RESOURCES += \
    qml.qrc
