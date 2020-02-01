#-------------------------------------------------
#
# Project created by QtCreator 2019-06-26T10:59:46
#
#-------------------------------------------------

QT       += widgets

TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(desktop)
DESTDIR = ../../../pluginlibs

include(../../../env.pri)



DEFINES += RUN_LOCAL

if(contains(DEFINES,RUN_LOCAL)){        #contains和{必须在同一行，否则报错

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \

LIBS          += -L$$PROJECT_COMPONENTLIBS -lcommoncomponent

} else {

INCLUDEPATH   +=  \
                 /usr/include/control-center/

LIBS          += -L/usr/lib/control-center/libs/ -lcommoncomponent

}

LIBS          += -L/usr/lib/ -lgsettings-qt

#DEFINES += QT_DEPRECATED_WARNINGS

#target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
#INSTALLS += target

SOURCES += \
        desktop.cpp

HEADERS += \
        desktop.h

FORMS += \
        desktop.ui
