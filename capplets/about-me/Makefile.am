SUBDIRS = icons

# This is used in UKUICC_CAPPLETS_CFLAGS
cappletname = about-me

ui_files = ukui-about-me-dialog.ui ukui-about-me-password.ui ukui-about-me-fingerprint.ui
Desktop_in_files = ukui-about-me.desktop.in

ukui_about_me_SOURCES =	 	\
	eel-alert-dialog.c		\
	eel-alert-dialog.h		\
	eel-gtk-macros.h		\
	ukui-about-me-password.c	\
	ukui-about-me-password.h	\
	e-image-chooser.c		\
	e-image-chooser.h		\
	ukui-about-me-fingerprint.c	\
	ukui-about-me-fingerprint.h	\
	$(MARSHALFILES)			\
	fingerprint-strings.h		\
	ukui-about-me.c

MARSHALFILES = marshal.c marshal.h
BUILT_SOURCES = $(MARSHALFILES)

marshal.h: fprintd-marshal.list
	@GLIB_GENMARSHAL@ --prefix=fprintd_marshal $< --header > $@
marshal.c: fprintd-marshal.list
	@GLIB_GENMARSHAL@ --prefix=fprintd_marshal $< --body --header > $@

bin_PROGRAMS = ukui-about-me

ukui_about_me_LDADD = $(UKUICC_CAPPLETS_LIBS)
ukui_about_me_LDFLAGS = -export-dynamic

@INTLTOOL_DESKTOP_RULE@

desktopdir = $(datadir)/applications
desktop_DATA = $(Desktop_in_files:.desktop.in=.desktop)

uidir   = $(pkgdatadir)/ui
ui_DATA = $(ui_files)

AM_CPPFLAGS = \
	$(UKUICC_CAPPLETS_CFLAGS) \
	$(LIBEBOOK_CFLAGS) \
	-DDATADIR="\"$(datadir)\"" \
	-DUKUICC_DATA_DIR="\"$(pkgdatadir)\"" \
	-DUKUICC_UI_DIR="\"$(uidir)\"" \
	-DUKUICC_PIXMAP_DIR="\"$(pkgdatadir)/pixmaps\"" \
	-DUKUILOCALEDIR="\"$(datadir)/locale\""

CLEANFILES = $(UKUICC_CAPPLETS_CLEANFILES) $(desktop_DATA) $(MARSHALFILES)
EXTRA_DIST = $(ui_files) fprintd-marshal.list

-include $(top_srcdir)/git.mk
