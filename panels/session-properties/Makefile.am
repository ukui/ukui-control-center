cappletname=sessionproperties
noinst_LTLIBRARIES=libsessionproperties.la

AUTOMAKE_OPTIONS=foreign
#INCLUDES= `pkg-config --cflags gtk+-2.0 gio-2.0 glib-2.0 dbus-glib-1`
#LIBS=`pkg-config --libs gtk+-2.0 gio-2.0 glib-2.0 dbus-glib-1`

AM_CPPFLAGS= \
	-DUIDIR="\"$(uidir)\""	\
	$(GTK_CFLAGS)	\
	$(GLIB_CFLAGS)	\
	$(DBUS_CFLAGS)	\
	$(NULL)

libsessionproperties_la_LIBADD =	\
	$(GTK_LIBS)	\
	$(GLIB_LIBS)	\
	$(DBUS_LIBS)	\
	$(NULL)

libsessionproperties_la_SOURCES =\
	eggdesktopfile.c	\
	eggdesktopfile.h	\
	gsm-app-dialog.c	\
	gsm-app-dialog.h	\
	gsm-properties-dialog.c	\
	gsm-properties-dialog.h	\
	gsm-util.c	\
	gsm-util.h	\
	gsp-app.c	\
	gsp-app.h	\
	gsp-app-manager.c	\
	gsp-app-manager.h	\
	gsp-keyfile.c	\
	gsp-keyfile.h	\
	gsp-main.c	\
	gsp-main.h
uidir= $(pkgdatadir)/ui
ui_DATA= main-table.ui program_properties.ui
-include $(top_srcdir)/git.mk

clean-local :
	rm -f *~ Makefile.in Makefile
