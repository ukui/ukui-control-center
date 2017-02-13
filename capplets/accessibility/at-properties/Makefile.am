bin_PROGRAMS = ukui-at-properties

ukui_at_properties_LDADD = $(AT_CAPPLET_LIBS) $(UKUICC_CAPPLETS_LIBS) $(top_builddir)/capplets/common/libcommon.la
ukui_at_properties_SOURCES = \
	main.c
ukui_at_properties_LDFLAGS = -export-dynamic

@INTLTOOL_DESKTOP_RULE@

desktopdir = $(datadir)/applications
Desktop_in_files = ukui-at-properties.desktop.in
desktop_DATA = $(Desktop_in_files:.desktop.in=.desktop)

pixmapdir  = $(pkgdatadir)/pixmaps
pixmap_DATA =					\
	at-startup.png				\
	at-support.png

uidir = $(pkgdatadir)/ui
ui_DATA = at-enable-dialog.ui

AM_CPPFLAGS   = $(AT_CAPPLET_CFLAGS)     \
             $(UKUICC_CAPPLETS_CFLAGS) \
	     -DUIDIR=\""$(uidir)"\" \
	     -DPIXMAPDIR=\""$(pixmapdir)"\" \
	     -DUKUICC_DATA_DIR="\"$(pkgdatadir)\"" \
	     -DUKUILOCALEDIR="\"$(datadir)/locale\""
CLEANFILES = $(UKUICC_CAPPLETS_CLEANFILES) $(Desktop_in_files) $(desktop_DATA)
EXTRA_DIST = $(ui_DATA) $(pixmap_DATA)

-include $(top_srcdir)/git.mk
