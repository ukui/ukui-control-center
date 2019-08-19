QT       += core dbus
QT       -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cc-qsysdbus-boot
TEMPLATE = app

CONFIG += console c++11 link_pkgconfig
CONFIG -= app_bundle

DESTDIR = .
INCLUDEPATH += .

HEADERS += \
    sysdbusregister.h

SOURCES += \
    main.cpp \
    sysdbusregister.cpp
