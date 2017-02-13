
AM_CPPFLAGS = $(FONT_VIEWER_CFLAGS) $(UKUICC_CAPPLETS_CFLAGS) -DDIRECTORY_DIR=\"$(directorydir)\" \
  -DUKUILOCALEDIR=\"$(datadir)/locale\"

bin_PROGRAMS = ukui-thumbnail-font ukui-font-viewer

ukui_thumbnail_font_LDADD = $(UKUICC_CAPPLETS_LIBS) $(FONT_VIEWER_LIBS)
ukui_thumbnail_font_SOURCES = ftstream-vfs.c ftstream-vfs.h font-thumbnailer.c totem-resources.c totem-resources.h

ukui_font_viewer_LDADD = $(UKUICC_CAPPLETS_LIBS) $(FONT_VIEWER_LIBS)
ukui_font_viewer_SOURCES = ftstream-vfs.c ftstream-vfs.h font-view.c

thumbnailerdir = $(datadir)/thumbnailers
thumbnailer_DATA = ukui-font-viewer.thumbnailer

desktopdir = $(datadir)/applications
desktop_in_files = ukui-font-viewer.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

install-data-local: install-desktop-database

install-desktop-database: install-desktopDATA
	update-desktop-database $(DESTDIR)$(desktopdir)

uninstall-local:
	rm -f $(DESTDIR)$(desktopdir)/mimeinfo.cache

@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_DIRECTORY_RULE@
@INTLTOOL_SCHEMAS_RULE@

CLEANFILES = $(desktop_in_files) $(desktop_DATA) \
  $(schemas_DATA) $(directory_DATA)

EXTRA_DIST = \
  $(thumbnailer_DATA) \
  ukui-font-viewer.desktop.in.in

-include $(top_srcdir)/git.mk
