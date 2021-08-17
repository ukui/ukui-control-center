include(../../../env.pri)
include($$PROJECT_COMPONENTSOURCE/label.pri)

QT       += widgets dbus KI18n KCoreAddons

TEMPLATE = lib
CONFIG += plugin \
          link_pkgconfig \
          c++11


TARGET = $$qtLibraryTarget(about)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

PKGCONFIG += gsettings-qt

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \

LIBS     += -L$$[QT_INSTALL_LIBS] -lgsettings-qt

QMAKE_CXXFLAGS *= -D_FORTIFY_SOURCE=2 -O2
#DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += \
    about.h

SOURCES += \
    about.cpp

FORMS += \
    about.ui

RESOURCES += \
    res/img.qrc

INSTALLS += target
