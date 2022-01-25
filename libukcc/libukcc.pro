QT += widgets

TARGET = ukcc
TEMPLATE = lib
DEFINES += LIBUKCC_LIBRARY

CONFIG += c++11

include(interface.pri)
include(widgets.pri)

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES +=

HEADERS += \
    libukcc_global.h \

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_LIBS]

    widgets.path = /usr/include/ukcc/widgets
    widgets.files = widgets/SwitchButton/*.h widgets/AddBtn/*.h widgets/Button/*.h \
                    widgets/CloseButton/*.h widgets/ComboBox/*.h widgets/ComboxFrame/*.h \
                    widgets/FlowLayout/*.h widgets/Frame/*.h widgets/HoverBtn/*.h \
                    widgets/HoverWidget/*.h widgets/ImageUtil/*.h widgets/InfoButton/*.h \
                    widgets/Label/*.h widgets/ListDelegate/*.h widgets/MaskWidget/*.h \
                    widgets/Uslider/*.h *.h

    interfaces.path = /usr/include/ukcc/interface
    interfaces.files =  *.h interface/*.h

    INSTALLS += widgets interfaces
}
!isEmpty(target.path): INSTALLS += target
