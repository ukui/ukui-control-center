# This is used in UKUICC_CAPPLETS_CFLAGS
cappletname = mouse

bin_PROGRAMS = ukui-mouse-properties

ukui_mouse_properties_LDADD = $(UKUICC_CAPPLETS_LIBS)
ukui_mouse_properties_SOURCES =	\
	ukui-mouse-properties.c

@INTLTOOL_DESKTOP_RULE@

pixmapdir  = $(pkgdatadir)/pixmaps
pixmap_DATA = \
	double-click-on.png		\
	double-click-off.png		\
	double-click-maybe.png

uidir   = $(pkgdatadir)/ui
ui_DATA = ukui-mouse-properties.ui

desktopdir = $(datadir)/applications
Desktop_in_files = ukui-settings-mouse.desktop.in
desktop_DATA = $(Desktop_in_files:.desktop.in=.desktop)

AM_CPPFLAGS = \
	$(UKUICC_CAPPLETS_CFLAGS) \
	-DUKUILOCALEDIR="\"$(datadir)/locale\"" \
	-DUKUICC_DATA_DIR="\"$(pkgdatadir)\"" \
	-DUKUICC_UI_DIR="\"$(uidir)\""
CLEANFILES = $(UKUICC_CAPPLETS_CLEANFILES) $(Desktop_in_files) $(desktop_DATA)
EXTRA_DIST = $(ui_DATA) $(pixmap_DATA)

-include $(top_srcdir)/git.mk
