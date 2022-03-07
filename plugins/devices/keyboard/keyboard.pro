#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------

include(../../../env.pri)

QT       += widgets x11extras KWindowSystem xml KGuiAddons KCoreAddons concurrent KConfigCore KConfigGui KI18n
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(keyboard)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \
                 /usr/include/KF5 \
                 /usr/include/xcb \

LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt -lX11

CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += libmatekbd \
                 gsettings-qt \


#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    keyboardcontrol.cpp \
    kbdlayoutmanager.cpp \
    keyboardmain.cpp \

HEADERS += \
    keyboardcontrol.h \
    kbdlayoutmanager.h \
    keyboardmain.h \

FORMS += \
    kbdlayoutmanager.ui \
    layoutmanager.ui

INSTALLS += target
