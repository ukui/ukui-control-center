TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets x11extras 
TARGET          = $$qtLibraryTarget(mouse-control)
DESTDIR         = ../../
INCLUDEPATH    += ../../..         \
                  /usr/include/kylinsdk \

LIBS          +=  -L/usr/lib/ -lmouseclient  -ltouchpadclient -lXi 
PKGCONFIG += gio-2.0 
CONFIG += no_keywords link_pkgconfig
    
HEADERS         += \
    mouse_setting.h

SOURCES         += \
    mouse_setting.cpp

FORMS            += \
    mouse_setting.ui

