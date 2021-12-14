include(../../../env.pri)

QT       += widgets dbus KXmlGui KGlobalAccel x11extras
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(shortcut)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH += \
               $$PROJECT_ROOTDIR \
               /usr/include/dconf

LIBS        += -L$$[QT_INSTALL_LIBS] -lgsettings-qt -ldconf

CONFIG += link_pkgconfig c++11

PKGCONFIG += gio-2.0 \
             gio-unix-2.0 \
             gsettings-qt \

DEFINES += QT_DEPRECATED_WARNINGS

#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    grab-x11.cpp  \
    addshortcutdialog.cpp \
    customlineedit.cpp \
    defineshortcutitem.cpp \
    doubleclicklineedit.cpp \
    getshortcutworker.cpp \
    clickfixlabel.cpp \
    keymap.cpp \
    realizeshortcutwheel.cpp \
    shortcut.cpp \
    shortcutline.cpp

HEADERS += \
    clickfixlabel.h \
    grab-x11.h  \
    addshortcutdialog.h \
    customlineedit.h \
    defineshortcutitem.h \
    doubleclicklineedit.h \
    getshortcutworker.h \
    keyentry.h \
    keymap.h \
    realizeshortcutwheel.h \
    shortcut.h \
    shortcutline.h

FORMS += \
    addshortcutdialog.ui \
    shortcut.ui

INSTALLS += target
