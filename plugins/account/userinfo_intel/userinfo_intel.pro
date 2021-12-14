#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T14:35:43
#
#-------------------------------------------------
include(../../../env.pri)

QT       += widgets dbus gui network

TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(userinfo_intel)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \

LIBS          += -L$$[QT_INSTALL_LIBS] -lcrypt -lpam

##加载gio库和gio-unix库
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0


#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    changefaceinteldialog.cpp \
    changegroupinteldialog.cpp \
    changephoneinteldialog.cpp \
    changepininteldialog.cpp \
    changepwdinteldialog.cpp \
    changetypeinteldialog.cpp \
    changevalidinteldialog.cpp \
    creategroupinteldialog.cpp \
    createuserinteldialog.cpp \
    definegroupitemintel.cpp \
    delgroupinteldialog.cpp \
    deluserinteldialog.cpp \
    digitalauthinteldialog.cpp \
    digitalphoneinteldialog.cpp \
    elipsemaskwidget.cpp \
    messageboxpowerintel.cpp \
    numbersbuttonintel.cpp \
    phoneauthinteldialog.cpp \
    picturetowhite.cpp \
    userinfo_intel.cpp \
    qtdbus/systemdbusdispatcher.cpp \
    qtdbus/userdispatcher.cpp \
    passwdcheckutil.cpp \
    auth-pam.cpp \
    passwordbar.cpp
#    changepasswd.cpp \

HEADERS += \
    changefaceinteldialog.h \
    changegroupinteldialog.h \
    changephoneinteldialog.h \
    changepininteldialog.h \
    changepwdinteldialog.h \
    changetypeinteldialog.h \
    changevalidinteldialog.h \
    creategroupinteldialog.h \
    createuserinteldialog.h \
    definegroupitemintel.h \
    delgroupinteldialog.h \
    deluserinteldialog.h \
    digitalauthinteldialog.h \
    digitalphoneinteldialog.h \
    elipsemaskwidget.h \
    messageboxpowerintel.h \
    numbersbuttonintel.h \
    phoneauthinteldialog.h \
    picturetowhite.h \
    userinfo_intel.h \
    qtdbus/systemdbusdispatcher.h \
    qtdbus/userdispatcher.h \
    passwdcheckutil.h \
    auth.h \
    auth-pam.h \
    passwordbar.h
#    changepasswd.h \

FORMS += \
    changefaceinteldialog.ui \
    changegroupinteldialog.ui \
    changephoneinteldialog.ui \
    changepininteldialog.ui \
    changepwdinteldialog.ui \
    changetypeinteldialog.ui \
    changevalidinteldialog.ui \
    creategroupinteldialog.ui \
    createuserinteldialog.ui \
    delgroupinteldialog.ui \
    deluserinteldialog.ui \
    userinfo_intel.ui

INSTALLS += target

RESOURCES += \
    changepwd.qrc
