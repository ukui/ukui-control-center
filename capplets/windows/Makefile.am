# This is used in UKUICC_CAPPLETS_CFLAGS
cappletname = window

bin_PROGRAMS = ukui-window-properties

ukui_window_properties_LDADD = $(UKUICC_CAPPLETS_LIBS)

ukui_window_properties_SOURCES = \
	ukui-metacity-support.h \
	ukui-metacity-support.c \
	ukui-window-properties.c

@INTLTOOL_DESKTOP_RULE@

desktopdir = $(datadir)/applications
Desktop_in_files = ukui-window-properties.desktop.in
desktop_DATA = $(Desktop_in_files:.desktop.in=.desktop)

AM_CPPFLAGS   = $(UKUICC_CAPPLETS_CFLAGS)					\
	     -DUIDIR=\""$(uidir)"\"  	\
	     -DUKUILOCALEDIR="\"$(datadir)/locale\""			\
	     -DPIXMAPDIR=\""$(pixmapdir)"\"

CLEANFILES = $(UKUICC_CAPPLETS_CLEANFILES) $(Desktop_in_files) $(desktop_DATA)

-include $(top_srcdir)/git.mk
