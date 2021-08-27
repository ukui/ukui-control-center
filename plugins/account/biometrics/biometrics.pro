include(../../../env.pri)
include($$PROJECT_COMPONENTSOURCE/switchbutton.pri)
include($$PROJECT_COMPONENTSOURCE/hoverwidget.pri)
include($$PROJECT_COMPONENTSOURCE/flowlayout.pri)
include($$PROJECT_COMPONENTSOURCE/imageutil.pri)
include($$PROJECT_COMPONENTSOURCE/closebutton.pri)
include($$PROJECT_COMPONENTSOURCE/label.pri)

QT       += widgets dbus gui

TEMPLATE = lib
CONFIG += c++11 plugin link_pkgconfig

TARGET = $$qtLibraryTarget(biometrics)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \
                 /usr/include/opencv4


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt -lpolkit-qt5-core-1

CONFIG        += link_pkgconfig \
                 C++11

PKGCONFIG     += gsettings-qt \
                 opencv4 \
                 gio-2.0

SOURCES += \
    biometrics.cpp \
    biometricswidget.cpp \
    biometricdeviceinfo.cpp \
    biometricproxy.cpp \
    biometricenroll.cpp \
    biometricmoreinfo.cpp \
    giodbus.cpp \
    servicemanager.cpp \
    namelabel.cpp \
    changefeaturename.cpp \
    changepwddialog.cpp \
    elipsemaskwidget.cpp \
    passwdcheckutil.cpp \
    pwdcheckthread.cpp \
    pwdchangethread.cpp 

HEADERS += \
    biometrics.h \
    biometricswidget.h \
    biometricdeviceinfo.h \
    biometricproxy.h \
    biometricenroll.h \
    biometricmoreinfo.h \
    giodbus.h \
    servicemanager.h \
    namelabel.h \
    changefeaturename.h \
    changepwddialog.h \
    elipsemaskwidget.h \
    passwdcheckutil.h \ 
    pwdcheckthread.h \
    pwdchangethread.h

FORMS += \
    biometricswidget.ui \
    biometricmoreinfo.ui \
    biometricenroll.ui \
    changefeaturename.ui \
    changepwddialog.ui


INSTALLS += target


