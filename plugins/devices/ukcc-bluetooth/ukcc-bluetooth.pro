#-------------------------------------------------
#
# Project created by QtCreator 2019-02-28T14:09:42
#
#-------------------------------------------------

include(../../../env.pri)
include($$PROJECT_COMPONENTSOURCE/switchbutton.pri)
include($$PROJECT_COMPONENTSOURCE/imageutil.pri)

QT       += widgets
QT       += core gui widgets dbus svg

TEMPLATE = lib
CONFIG += plugin \
          += c++11 \
          link_pkgconfig


PKGCONFIG     += gsettings-qt gio-2.0
LIBS += -L /usr/lib/x86_64-linux-gnur -l KF5BluezQt -lgio-2.0 -lglib-2.0


TARGET = $$qtLibraryTarget(ukcc-bluetooth)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH += \
               $$PROJECT_COMPONENTSOURCE \
               $$PROJECT_ROOTDIR \

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
#           QT_NO_WARNING_OUTPUT \
#           QT_NO_DEBUG_OUTPUT
exists("/etc/apt/ota_version")
{
    DEFINES += DEVICE_IS_INTEL
}

SOURCES += \
    bluetooth.cpp \
    bluetoothmain.cpp \
    bluetoothnamelabel.cpp \
    config.cpp \
    devicebase.cpp \
    deviceinfoitem.cpp \
    devremovedialog.cpp \
    devrenamedialog.cpp \
    intelcustomizenamelabel.cpp \
    inteldeviceinfoitem.cpp \
    intelmsgbox.cpp \
    loadinglabel.cpp \
    ukccbluetoothconfig.cpp

HEADERS += \
    bluetooth.h \
    bluetoothmain.h \
    bluetoothnamelabel.h \
    config.h \
    devicebase.h \
    deviceinfoitem.h \
    devremovedialog.h \
    devrenamedialog.h \
    intelcustomizenamelabel.h \
    inteldeviceinfoitem.h \
    intelmsgbox.h \
    loadinglabel.h \
    ukccbluetoothconfig.h

INSTALLS += target

DISTFILES += \
    icon-bluetooth/help.png \
    icon-bluetooth/ukui-bluetooth-delete-selected.svg \
    icon-bluetooth/ukui-bluetooth-delete.png \
    icon-bluetooth/ukui-bluetooth-delete.svg \
    icon-bluetooth/ukui-bluetooth-send-file-selected.svg \
    icon-bluetooth/ukui-bluetooth-send-file.png \
    icon-bluetooth/ukui-bluetooth-send-file.svg

RESOURCES += \
    image.qrc
