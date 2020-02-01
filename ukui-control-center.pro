TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = \
    commonComponent \
    registeredQDbus \
    plugins\
    shell \


TRANSLATIONS += \
    shell/res/i18n/zh_CN.ts
