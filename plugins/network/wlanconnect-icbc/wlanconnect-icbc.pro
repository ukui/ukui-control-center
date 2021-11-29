QT       += widgets network dbus gui core
TEMPLATE = lib
CONFIG += plugin

include(../../../env.pri)
include($$PROJECT_COMPONENTSOURCE/hoverbtn.pri)
include($$PROJECT_COMPONENTSOURCE/label.pri)

include(../component/switchbutton.pri)
include(../component/infobutton.pri)
include(../component/drownlabel.pri)

TARGET = $$qtLibraryTarget(wlanconnect-icbc)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \

LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt

CONFIG += c++11 \
          link_pkgconfig \

PKGCONFIG += gsettings-qt \

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    deviceframe.cpp \
#    drownlabel.cpp \
    itemframe.cpp \
    wlanconnect.cpp \
    wlanitem.cpp

HEADERS += \
    deviceframe.h \
#    drownlabel.h \
    itemframe.h \
    wlanconnect.h \
    wlanitem.h

FORMS += \
    wlanconnect.ui

INSTALLS += target \
