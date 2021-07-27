QT += core gui
CONFIG += c++11

INCLUDEPATH += -I $$PWD/

PKGCONFIG += \
        glib-2.0\
        gio-2.0 libxklavier

SOURCES += \
    $$PWD/clib-syslog.c

HEADERS += \
    $$PWD/clib-syslog.h \

