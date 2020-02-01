#-------------------------------------------------
#
# Project created by QtCreator 2019-02-20T15:36:43
#
#-------------------------------------------------

QT            += widgets
TEMPLATE = lib
CONFIG        += plugin

TARGET = $$qtLibraryTarget(defaultapp)
DESTDIR = ../../../pluginlibs

##加载gio库和gio-unix库，用于处理desktop文件
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0

include(../../../env.pri)

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \
                 /usr/include/ukuisdk

LIBS          += -L/usr/lib/ -ldefaultprograms \
                 -L$$PROJECT_COMPONENTLIBS -lcommoncomponent


target.path = $$[QT_INSTALL_PREFIX]/lib/control-center/plugins/
INSTALLS += target

SOURCES += \
        defaultapp.cpp \
    addappdialog.cpp \
#    component/custdomcombobox.cpp

HEADERS += \
        defaultapp.h \
    addappdialog.h \
#    component/custdomcombobox.h

FORMS += \
    defaultapp.ui \
    addappdialog.ui
