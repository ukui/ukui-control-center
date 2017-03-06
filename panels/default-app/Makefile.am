cappletname=default_app
noinst_LTLIBRARIES=libdefaultapp.la

AUTOMAKE_OPTIONS = foreign
#INCLUDES= `pkg-config --cflags gtk+-2.0 gio-2.0 glib-2.0 gdk-pixbuf-2.0`
#LIBS= `pkg-config --libs gtk+-2.0 gio-2.0 glib-2.0 gdk-pixbuf-2.0`
AM_CPPFLAGS =	\
	$(GTK_CFLAGS)	\
	$(GLIB_CFLAGS)	\
	$(GTK_PIXBUF_CFLAGS)	\
	$(NULL)

libdefaultapp_la_LIBADD = 	\
	$(GTK_LIBS)
	$(GLIB_LIBS)
	$(GTK_PIXBUF_LIBS)
	$(NULL)

libdefaultapp_la_SOURCES= \
	default-app.c	\
	default-app.h	\
-include $(top_srcdir)/git.mk

clean-local :
	rm -f *~ Makefile.in Makefile
