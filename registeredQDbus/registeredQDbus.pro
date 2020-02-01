QT       += core dbus
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = launchSysDbus
TEMPLATE = app

CONFIG += console c++11 link_pkgconfig
CONFIG -= app_bundle

DESTDIR = .
INCLUDEPATH += .

target.source += $$TARGET
target.path = /usr/bin
INSTALLS += \
    target

HEADERS += \
    sysdbusregister.h

SOURCES += \
    main.cpp \
    sysdbusregister.cpp
