TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets x11extras 
INCLUDEPATH    += ../../..        \
                    /usr/include/kylinsdk \


LIBS          +=  -L/usr/lib/ -lkeyboardclient -linterfaceclient -lxklavier
PKGCONFIG += gio-2.0

CONFIG += no_keywords link_pkgconfig
    
HEADERS         += keyboard_settings.h \
                    treeitem.h \
                    treemodel.h \
		    keybinding_view.h \
		    keyboard_xkb.h

SOURCES         = keyboard_settings.cpp \
                    treeitem.cpp \
                    treemodel.cpp \
		    keybinding_view.cpp \
		    keyboard_xkb.cpp

FORMS            = keyboard.ui
TARGET          = $$qtLibraryTarget(keyboard-control)
DESTDIR         = ../../




