include(../../../env.pri)
include($$PROJECT_COMPONENTSOURCE/label.pri)

QT       += widgets dbus KI18n KCoreAddons \
             widgets dbus concurrent
TEMPLATE = lib
CONFIG += plugin \
          link_pkgconfig

PKGCONFIG     += gsettings-qt


TARGET = $$qtLibraryTarget(about)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \
                 $$PROJECT_COMPONENTSOURCE
#DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += \
    HpQRCodeInterface.h \
    about.h \
    trialdialog.h

SOURCES += \
    about.cpp \
    trialdialog.cpp

FORMS +=

RESOURCES += \
    res/img.qrc

INSTALLS += target
