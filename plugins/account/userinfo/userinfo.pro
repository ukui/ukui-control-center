#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T14:35:43
#
#-------------------------------------------------
include(../../../env.pri)
include($$PROJECT_COMPONENTSOURCE/switchbutton.pri)
include($$PROJECT_COMPONENTSOURCE/hoverwidget.pri)
include($$PROJECT_COMPONENTSOURCE/flowlayout.pri)
include($$PROJECT_COMPONENTSOURCE/imageutil.pri)

QT       += widgets dbus gui network

TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(userinfo)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \

LIBS          += -L$$[QT_INSTALL_LIBS] -lcrypt -lpam

##加载gio库和gio-unix库
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0


#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    changegroupdialog.cpp \
    changepindialog.cpp \
    changephonedialog.cpp \
    changevaliddialog.cpp \
    creategroupdialog.cpp \
    definegroupitem.cpp \
    delgroupdialog.cpp \
    digitalphonedialog.cpp \
    elipsemaskwidget.cpp \
    messageboxpower.cpp \
    picturetowhite.cpp \
    userinfo.cpp \
    qtdbus/systemdbusdispatcher.cpp \
    changepwddialog.cpp \
    qtdbus/userdispatcher.cpp \
    changetypedialog.cpp \
    changefacedialog.cpp \
    deluserdialog.cpp \
    createuserdialog.cpp \
    passwdcheckutil.cpp \
    auth-pam.cpp \
    passwordbar.cpp \
    numbersbutton.cpp \
#    changepasswd.cpp \
    digitalauthdialog.cpp \
    phoneauthdialog.cpp

HEADERS += \
    changegroupdialog.h \
    changepindialog.h \
    changephonedialog.h \
    changevaliddialog.h \
    creategroupdialog.h \
    definegroupitem.h \
    delgroupdialog.h \
    digitalphonedialog.h \
    elipsemaskwidget.h \
    messageboxpower.h \
    picturetowhite.h \
    userinfo.h \
    qtdbus/systemdbusdispatcher.h \
    changepwddialog.h \
    qtdbus/userdispatcher.h \
    changetypedialog.h \
    changefacedialog.h \
    deluserdialog.h \
    createuserdialog.h \
    passwdcheckutil.h \
    auth.h \
    auth-pam.h \
    passwordbar.h \
    numbersbutton.h \
#    changepasswd.h \
    digitalauthdialog.h \
    phoneauthdialog.h

FORMS += \
    changegroupdialog.ui \
    changephonedialog.ui \
    changevaliddialog.ui \
    changepindialog.ui \
    creategroupdialog.ui \
    delgroupdialog.ui \
    userinfo.ui \
    changepwddialog.ui \
    changetypedialog.ui \
    changefacedialog.ui \
    deluserdialog.ui \
    createuserdialog.ui

INSTALLS += target

RESOURCES += \
    changepwd.qrc
