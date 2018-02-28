cappletname=display_app
noinst_LTLIBRARIES=libdisplayapp.la

AUTOMAKE_OPTIONS=foreign
#INCLUDES=`pkg-config --cflags gtk+-2.0 glib-2.0 dbus-glib-1 gio-2.0 mate-settings-daemon mate-desktop-2.0`
#LIBS=`pkg-config --libs gtk+-2.0 glib-2.0 dbus-glib-1 gio-2.0 mate-settings-daemon mate-desktop-2.0`

sbin_PROGRAMS = ukui-display-properties-install-systemwide

ukui_display_properties_install_systemwide_SOURCES =	\
	ukui-display-properties-install-systemwide.c

ukui_display_properties_install_systemwide_LDADD =	\
	$(GLIB_LIBS)

polkit_policydir = $(datadir)/polkit-1/actions
dist_polkit_policy_DATA =					\
	org.ukuicc.randr.policy

@INTLTOOL_POLICY_RULE@

AM_CPPFLAGS   = $(DISPLAY_CAPPLET_CFLAGS) \
             $(MATECC_CAPPLETS_CFLAGS) \
	     -DSBINDIR="\"$(sbindir)\"" \
	     -DUIDIR="\"$(uidir)\"" \
	     -DMATELOCALEDIR="\"$(datadir)/locale\"" \
	     -DMATECC_DATA_DIR="\"$(pkgdatadir)\""	\
		$(GTK_CFLAGS)	\
		$(GLIB_CFLAGS)	\
		$(DBUS_LIB_CFLAGS)	\
		$(MATE_SD_CFLAGS)	\
		$(MATE_DESKTOP_CFLAGS)	\
		$(NULL)

libdisplayapp_la_LIBADD =	\
	$(GTK_LIBS)	\
	$(GLIB_LIBS)	\
	$(DBUS_LIB_LIBS)	\
	$(MATE_SD_LIBS)	\
	$(MATE_DESKTOP_LIBS)	\
	$(NULL)

libdisplayapp_la_SOURCES=\
	foo-marshal.c \
	foo-marshal.h \
	scrollarea.c \
	scrollarea.h \
	xrandr-capplet.c \
	xrandr-capplet.h 

EXTRA_DIST = org.ukuicc.randr.policy.in
DISTCLEANFILES = org.ukuicc.randr.policy

-include $(top_srcdir)/git.mk

clean-local :
	rm -f *~ Makefile.in Makefile
