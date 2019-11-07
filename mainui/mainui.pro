#-------------------------------------------------
#
# Project created by QtCreator 2019-02-14T10:13:22
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ukui-control-center
TEMPLATE = app

DESTDIR = ..
INCLUDEPATH += .

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += link_pkgconfig

PKGCONFIG += gtk+-3.0

include(../plugins/pluginsComponent/pluginsComponent.pri)

inst1.files += ukui-control-center.desktop
inst1.path = /usr/share/applications
target.source += $$TARGET
target.path = /usr/bin
INSTALLS += inst1 \
    target

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    modulepagewidget.cpp \
    mainpagewidget.cpp \
    mainComponent/custdomlabel.cpp \
    mainComponent/custdomframe.cpp \
    mainComponent/listwidgetitem.cpp \
    framelessExtended/framelesshandle.cpp \
    framelessExtended/cursorposcalculator.cpp \
    framelessExtended/widgethandlerealize.cpp

HEADERS += \
        mainwindow.h \
        interface.h \
    modulepagewidget.h \
    mainpagewidget.h \
    mainComponent/custdomlabel.h \
    mainComponent/custdomframe.h \
    mainComponent/listwidgetitem.h \
    kpplication.h \
    framelessExtended/framelesshandle.h \
    framelessExtended/framelesshandleprivate.h \
    framelessExtended/cursorposcalculator.h \
    framelessExtended/widgethandlerealize.h

FORMS += \
        mainwindow.ui \
    modulepagewidget.ui \
    mainpagewidget.ui

RESOURCES += \
    res/img.qrc

DISTFILES +=

INCLUDEPATH += qtsingleapplication
DEPENDPATH += qtsingleapplication

SOURCES += qtsingleapplication/qtsingleapplication.cpp qtsingleapplication/qtlocalpeer.cpp
HEADERS += qtsingleapplication/qtsingleapplication.h qtsingleapplication/qtlocalpeer.h
