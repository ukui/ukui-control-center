include(../../../env.pri)

QT       += widgets dbus KI18n KCoreAddons \
             widgets dbus concurrent svg
TEMPLATE = lib
CONFIG += plugin \
          link_pkgconfig

PKGCONFIG     += gsettings-qt

TARGET = $$qtLibraryTarget(about)
DESTDIR = ../..
target.path = $${PLUGIN_INSTALL_DIRS}

INCLUDEPATH   +=  \
                 $$PROJECT_ROOTDIR \

#DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += \
    HpQRCodeInterface.h \
    about.h \
    hostnamedialog.h \
    privacydialog.h \
    statusdialog.h \
    trialdialog.h

SOURCES += \
    about.cpp \
    hostnamedialog.cpp \
    privacydialog.cpp \
    statusdialog.cpp \
    trialdialog.cpp

FORMS +=

RESOURCES += \
    res/img.qrc

INSTALLS += target
