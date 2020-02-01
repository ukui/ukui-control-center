QT += gui widgets

TEMPLATE = lib
#CONFIG += staticlib //静态库
#CONFIG += plugin   //插件库

include(switchbutton.pri)
include(combobox.pri)
include(hoverwidget.pri)
include(../env.pri)

TARGET = $$qtLibraryTarget(commoncomponent)
DESTDIR = $$PROJECT_COMPONENTLIBS


inst1.files += SwitchButton/switchbutton.h
inst1.path = /usr/include/control-center/
inst2.files += HoverWidget/hoverwidget.h
inst2.path = /usr/include/control-center/
inst3.files += ComboBox/combobox.h
inst3.path = /usr/include/control-center/

target.source += $$TARGET
target.path = /usr/lib/control-center/libs/

INSTALLS +=  \
            inst1 \
            inst2 \
            inst3 \
            target \
