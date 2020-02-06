#-------------------------------------------------
#
# Project created by QtCreator 2019-12-21T09:41:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ukui-control-center
DESTDIR = ..
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(../env.pri)

inst1.files += ukui-control-center.desktop
inst1.path = /usr/share/applications
inst2.files += $$PROJECT_ROOTDIR/pluginlibs/
inst2.path = /usr/lib/control-center/

target.source += $$TARGET
target.path = /usr/bin

INSTALLS +=  \
            inst1 \
            inst2 \
            target \

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    framelessExtended/cursorposcalculator.cpp \
    framelessExtended/framelesshandle.cpp \
    framelessExtended/widgethandlerealize.cpp \
    homepagewidget.cpp \
    modulepagewidget.cpp \
    utils/keyvalueconverter.cpp \
    component/leftwidgetitem.cpp \
    component/clicklabel.cpp \
    utils/functionselect.cpp \
    component/hoverwidget.cpp

HEADERS += \
        mainwindow.h \
        interface.h \
    framelessExtended/cursorposcalculator.h \
    framelessExtended/framelesshandle.h \
    framelessExtended/framelesshandleprivate.h \
    framelessExtended/widgethandlerealize.h \
    homepagewidget.h \
    modulepagewidget.h \
    utils/keyvalueconverter.h \
    component/leftwidgetitem.h \
    component/clicklabel.h \
    utils/functionselect.h \
    component/hoverwidget.h

FORMS += \
        mainwindow.ui \
    homepagewidget.ui \
    modulepagewidget.ui

RESOURCES += \
    res/resfile.qrc
