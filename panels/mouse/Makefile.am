cappletname=mouse_app
noinst_LTLIBRARIES=libmouseapp.la

AUTOMAKE_OPTIONS=foreign
#INCLUDES=`pkg-config --cflags gtk+-2.0 glib-2.0 dbus-glib-1 gio-2.0 mate-settings-daemon x11 xi`
#LIBS=`pkg-config --libs gtk+-2.0 glib-2.0 dbus-glib-1 gio-2.0 mate-settings-daemon x11 xi`

AM_CPPFLAGS =	\
	$(GTK_CFLAGS)	\
	$(GLIB_CFLAGS)	\
	$(DBUS_CFLAGS)	\
	$(UKUI_SD_CFLAGS)	\
	$(X11_CFLAGS)	\
	$(XI_CFLAGS)	\
	$(NULL)

libmouseapp_la_LIBADD =	\
	$(GTK_LIBS)	\
	$(GLIB_LIBS)	\
	$(DBUS_LIBS)	\
	$(UKUI_SD_LIBS)	\
	$(X11_LIBS)	\
	$(XI_LIBS)	\
	$(NULL)

libmouseapp_la_SOURCES=\
    mouse-setting.c \
    mouse-setting.h
-include $(top_srcdir)/git.mk
pixmapdir = $(pkgdatadir)/pixmaps
pixmap_DATA = double-click-on.png double-click-maybe.png double-click-off.png
clean-local :
	rm -f *~ Makefile.in Makefile
