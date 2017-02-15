SUBDIRS = data

# This is used in UKUICC_CAPPLETS_CFLAGS
cappletname = appearance

bin_PROGRAMS = ukui-appearance-properties

ukui_appearance_properties_SOURCES = \
	appearance.h \
	appearance-desktop.c \
	appearance-desktop.h \
	appearance-font.c \
	appearance-font.h \
	appearance-main.c \
	appearance-themes.c \
	appearance-themes.h \
	appearance-style.c \
	appearance-style.h \
	appearance-ui.c \
	appearance-ui.h \
	appearance-support.c \
	appearance-support.h \
	ukui-wp-info.c \
	ukui-wp-info.h \
	ukui-wp-item.c \
	ukui-wp-item.h \
	ukui-wp-xml.c \
	ukui-wp-xml.h \
	theme-installer.c \
	theme-installer.h \
	theme-save.c \
	theme-save.h \
	theme-util.c \
	theme-util.h

AM_CFLAGS = -DMATE_DESKTOP_USE_UNSTABLE_API

ukui_appearance_properties_LDADD = \
	$(top_builddir)/libwindow-settings/libukui-window-settings.la \
	$(top_builddir)/capplets/common/libcommon.la \
	$(UKUICC_CAPPLETS_LIBS) \
	$(FONT_CAPPLET_LIBS) \
	$(UKWM_LIBS)
ukui_appearance_properties_LDFLAGS = -export-dynamic

gtkbuilderdir = $(pkgdatadir)/ui
pixmapdir = $(pkgdatadir)/pixmaps
wallpaperdir = $(datadir)/ukui-background-properties

AM_CPPFLAGS = \
	$(UKWM_CFLAGS) \
	$(UKUICC_CAPPLETS_CFLAGS) \
	$(FONT_CAPPLET_CFLAGS) \
	-DUKUILOCALEDIR="\"$(datadir)/locale\"" \
	-DUKUICC_DATA_DIR="\"$(pkgdatadir)\"" \
	-DUKUICC_GTKBUILDER_DIR="\"$(gtkbuilderdir)\"" \
	-DUKUICC_PIXMAP_DIR="\"$(pixmapdir)\"" \
	-DWALLPAPER_DATADIR="\"$(wallpaperdir)\""

CLEANFILES = $(UKUICC_CAPPLETS_CLEANFILES)

-include $(top_srcdir)/git.mk
