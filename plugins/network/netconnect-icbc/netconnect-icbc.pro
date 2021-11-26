QT       += widgets network dbus gui core
TEMPLATE = lib
CONFIG += plugin

include(../component/switchbutton.pri)
include(../component/infobutton.pri)
include(../component/drownlabel.pri)

include(../../../env.pri)
include($$PROJECT_COMPONENTSOURCE/hoverbtn.pri)
include($$PROJECT_COMPONENTSOURCE/label.pri)

TARGET = $$qtLibraryTarget(netconnect-icbc)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \
                 /usr/include/ukcc/interface \
                 /usr/include/ukcc/widgets

LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt

CONFIG += c++11 \
          link_pkgconfig \

PKGCONFIG += gsettings-qt \

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    addnetbtn.cpp \
    deviceframe.cpp \
#    drownlabel.cpp \
    itemframe.cpp \
    lanitem.cpp \
    netconnect.cpp

HEADERS += \
    addnetbtn.h \
    deviceframe.h \
#    drownlabel.h \
    itemframe.h \
    lanitem.h \
    netconnect.h

FORMS += \
    netconnect.ui

INSTALLS += target \
            trans
