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
