DEFINES += SP2

TEMPLATE = subdirs

SUBDIRS = devices/printer \
          devices/mouse \
          system/about \
          system/power \
          personalized/wallpaper \
          personalized/theme \
          personalized/screenlock \
          network/vpn \
          network/proxy \
          time-language/datetime \
          time-language/area

if(contains(DEFINES, SP2)){
SUBDIRS += devices/shortcut \
           devices/keyboard \
           devices/touchpad \
           application/defaultapp \
           application/autoboot \
           system/notice \
           system/display \
           system/vino \
           personalized/fonts \
           personalized/screensaver \
           account/userinfo \
           security-updates/backup \
}

if(contains(DEFINES, PANGUW)){
SUBDIRS += system/display_hw \
           system/vino_hw \
}

if(contains(DEFINES, MAVIS)){
SUBDIRS += system/display \
           system/backup_intel \
           account/userinfo_intel \
}
