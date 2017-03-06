cappletname=network_proxy
noinst_LTLIBRARIES=libnetworkproxy.la

AUTOMAKE_OPTIONS=foreign
#INCLUDES=`pkg-config --cflags gtk+-2.0 glib-2.0 gio-2.0`
#LIBS=`pkg-config --libs gtk+-2.0 glib-2.0 gio-2.0`

AM_CPPFLAGS =	\
	-DUIDIR="\"$(uidir)\""	\
	$(GTK_CFLAGS)	\
	$(GLIB_CFLAGS)	\
	$(NULL)

libnetworkproxy_la_LIBADD =	\
	$(GTK_LIBS)	\
	$(GLIB_LIBS)	\
	$(NULL)

uidir = $(pkgdatadir)/ui
ui_DATA = network.ui

libnetworkproxy_la_SOURCES=\
    network-proxy.c \
    network-proxy.h \
-include $(top_srcdir)/git.mk

clean-local :
	rm -f *~ Makefile.in Makefile
