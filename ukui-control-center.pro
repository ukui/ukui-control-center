TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = \
    changeOtherUserPwd \
    changeUserPwd \
    checkUserPwdWithPAM \
    registeredQDbus \
    plugins\
    registeredSession \
    shell \
    libukcc \
    group-manager-server \
#    tastenbrett \


TRANSLATIONS += \
    shell/res/i18n/zh_CN.ts \
    shell/res/i18n/tr.ts \
    shell/res/i18n/en_US.ts \


# Automating generation .qm files from .ts files
CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}

isEmpty(PREFIX) {
    PREFIX = /usr
}

qm_files.path = $${PREFIX}/share/ukui-control-center/shell/res/i18n/
qm_files.files = shell/res/i18n/*.qm

search_file.path = $${PREFIX}/share/ukui-control-center/shell/res/
search_file.files = shell/res/search.xml

INCLUDEPATH += /usr/lib/gcc/aarch64-linux-gnu/9/include/

INSTALLS += qm_files \
            search_file

HEADERS += \
    shell/utils/mthread.h \
    shell/utils/xatom-helper.h

SOURCES += \
    shell/utils/mthread.cpp \
    shell/utils/xatom-helper.cpp
