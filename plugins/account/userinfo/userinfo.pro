#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T14:35:43
#
#-------------------------------------------------
include(../../../env.pri)

include($$PROJECT_COMPONENTSOURCE/hoverwidget.pri)
include($$PROJECT_COMPONENTSOURCE/flowlayout.pri)
include($$PROJECT_COMPONENTSOURCE/imageutil.pri)
include($$PROJECT_COMPONENTSOURCE/closebutton.pri)


include($$PROJECT_COMPONENTSOURCE/addbtn.pri)
include($$PROJECT_COMPONENTSOURCE/comboxframe.pri)
include($$PROJECT_COMPONENTSOURCE/label.pri)
include($$PROJECT_COMPONENTSOURCE/switchbutton.pri)

QT       += widgets dbus gui

TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(userinfo)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \

LIBS          += -L$$[QT_INSTALL_LIBS] -lcrypt -lpolkit-qt5-core-1 -lpam -lkysec   -lpwquality
DEFINES += ENABLEPQ
DEFINES += WITHKYSEC

##加载gio库和gio-unix库
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \
                 gsettings-qt


#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    changegroupdialog.cpp \
    changeuserlogo.cpp \
    changeusernickname.cpp \
    changeuserpwd.cpp \
    changeusertype.cpp \
    creategroupdialog.cpp \
    createusernew.cpp \
    definegroupitem.cpp \
    deleteuserexists.cpp \
    delgroupdialog.cpp \
    editgroupdialog.cpp \
    elipsemaskwidget.cpp \
    pwdcheckthread.cpp \
    userinfo.cpp \
    qtdbus/systemdbusdispatcher.cpp \
    qtdbus/userdispatcher.cpp \
    passwdcheckutil.cpp \
    utilsforuserinfo.cpp

HEADERS += \
    changegroupdialog.h \
    changeuserlogo.h \
    changeusernickname.h \
    changeuserpwd.h \
    changeusertype.h \
    creategroupdialog.h \
    createusernew.h \
    definegroupitem.h \
    deleteuserexists.h \
    delgroupdialog.h \
    editgroupdialog.h \
    elipsemaskwidget.h \
    loginedusers.h \
    pwdcheckthread.h \
    userinfo.h \
    qtdbus/systemdbusdispatcher.h \
    qtdbus/userdispatcher.h \
    passwdcheckutil.h \
    utilsforuserinfo.h

FORMS += \
    changegroupdialog.ui \
    delgroupdialog.ui \
    editgroupdialog.ui \
    creategroupdialog.ui \
    userinfo.ui \

INSTALLS += target
