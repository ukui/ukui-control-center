WM_MODULE_DIR=$(libdir)/window-manager-settings

AM_CPPFLAGS = 								\
	-DUKUILOCALEDIR="\"$(datadir)/locale\""			\
	-DUKUI_ICONDIR=\""$(datadir)/pixmaps"\"			\
	-DG_LOG_DOMAIN=\"capplet-common\"				\
	-DUKUI_WM_PROPERTY_PATH=\"$(datadir)/ukui/wm-properties\"	\
	-DUKUI_WINDOW_MANAGER_MODULE_PATH=\""$(WM_MODULE_DIR)"\"	\
	-I$(top_srcdir)/						\
	@CAPPLET_CFLAGS@						\
	-DUKWM_THEME_DIR=\""$(datadir)/themes"\"

lib_LTLIBRARIES = libukui-window-settings.la

libukui_window_settings_la_LDFLAGS = 	\
	-export_dynamic			\
	-no-undefined			\
        -version-info 1:0:0

libukui_window_settings_la_LIBADD = $(UKUICC_LIBS)

libukui_window_settings_la_SOURCES = \
	ukui-window-manager.c	\
	ukui-window-manager.h	\
	ukui-wm-manager.c	\
	ukui-wm-manager.h

libukui_window_settingsincludedir = $(includedir)/ukui-window-settings-2.0

libukui_window_settingsinclude_HEADERS =	\
	ukui-window-manager.h			\
	ukui-wm-manager.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ukui-window-settings-2.0.pc

EXTRA_DIST = 					\
	ukui-window-settings-2.0.pc.in

wuks_flags = -export_dynamic -avoid-version -module
wuksdir = $(WM_MODULE_DIR)

wuks_LTLIBRARIES =			\
	libukwm.la

libukwm_la_SOURCES = 		\
	ukwm-window-manager.c	\
	ukwm-window-manager.h

libukwm_la_LDFLAGS = $(wms_flags)
libukwm_la_LIBADD = 		\
	$(UKUICC_LIBS)			\
	libukui-window-settings.la

-include $(top_srcdir)/git.mk
