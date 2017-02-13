# This is used in UKUICC_CAPPLETS_CFLAGS
cappletname = ukui-default-applications-properties

bin_PROGRAMS = ukui-default-applications-properties

ukui_default_applications_properties_LDADD = $(UKUICC_CAPPLETS_LIBS)
ukui_default_applications_properties_SOURCES = \
	ukui-da-capplet.c ukui-da-capplet.h

@INTLTOOL_DESKTOP_RULE@

uidir = $(pkgdatadir)/ui
dist_ui_DATA = ukui-default-applications-properties.ui

desktopdir = $(datadir)/applications
Desktop_in_files = ukui-default-applications-properties.desktop.in
desktop_DATA = $(Desktop_in_files:.desktop.in=.desktop)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ukui-default-applications.pc

AM_CPPFLAGS = \
	$(UKUICC_CAPPLETS_CFLAGS) \
	$(DEFAULT_APPLICATIONS_CAPPLET_CFLAGS) \
	-DAPPLICATIONSDIR=\""$(datadir)/applications"\"\
	-DUKUILOCALEDIR=\""$(datadir)/locale"\"\
	-DUKUICC_UI_DIR=\""$(uidir)"\"

CLEANFILES = $(UKUICC_CAPPLETS_CLEANFILES) $(Desktop_in_files) $(desktop_DATA) $(autostart_DATA)
EXTRA_DIST = ukui-default-applications.pc.in ukui-default-applications-properties.desktop.in.in

-include $(top_srcdir)/git.mk
