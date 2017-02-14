EXTRA_DIST =

AM_CPPFLAGS = \
	-DUKUICC_DATA_DIR="\"$(pkgdatadir)\""				\
	-DUKUILOCALEDIR="\"$(datadir)/locale\""			\
	-DGTK_ENGINE_DIR="\"$(GTK_ENGINE_DIR)\"" 			\
	-DG_LOG_DOMAIN=\"capplet-common\"				\
	-DINSTALL_PREFIX=\"$(prefix)\"					\
	-I$(top_srcdir)							\
	-I$(top_srcdir)/libwindow-settings				\
	-DPIXMAP_DIR=\""$(datadir)/ukui-control-center/pixmaps"\"	\
	$(CAPPLET_CFLAGS)						\
	$(DBUS_CFLAGS)							\
	$(MATE_DESKTOP_CFLAGS)						\
	$(MARCO_CFLAGS)						\
	$(GSD_DBUS_CFLAGS)						\
	$(GIO_CFLAGS)						\
	$(DCONF_CFLAGS)


noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = \
	activate-settings-daemon.c	\
	activate-settings-daemon.h	\
	capplet-stock-icons.c		\
	capplet-stock-icons.h		\
	capplet-util.c			\
	capplet-util.h			\
	dconf-util.c				\
	dconf-util.h				\
	file-transfer-dialog.c		\
	file-transfer-dialog.h		\
	ukui-theme-apply.c		\
	ukui-theme-apply.h 		\
	ukui-theme-info.c		\
	ukui-theme-info.h		\
	gtkrc-utils.c			\
	gtkrc-utils.h			\
	theme-thumbnail.c		\
	theme-thumbnail.h		\
	wm-common.c			\
	wm-common.h

libcommon_la_LIBADD =							\
	$(top_builddir)/libwindow-settings/libukui-window-settings.la  \
	$(MARCO_LIBS)						\
	$(DBUS_LIBS)							\
	$(MATE_DESKTOP_LIBS)						\
	$(GIO_LIBS)									\
	$(DCONF_LIBS)

ukui_theme_test_SOURCES = \
	ukui-theme-test.c

ukui_theme_test_LDADD = 						\
	libcommon.la							\
	$(UKUICC_CAPPLETS_LIBS)						\
	$(UKUICC_LIBS)

noinst_PROGRAMS = \
	ukui-theme-test

-include $(top_srcdir)/git.mk
