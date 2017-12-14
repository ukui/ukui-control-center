cappletname=appearance
noinst_LTLIBRARIES=libappearance.la

AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = \
	-DUIDIR="\"$(uidir)\""	\
	@PACKAGE_CFLAGS@	\
	-fPIC	\
	-I$(srcdir)/../../shell/	\
	$(GLIB_CFLAGS)	\
	$(XFT_CFLAGS)	\
	$(LIBXML_CFLAGS)	\
	$(MATE_DESKTOP_CFLAGS)	\
	$(X11_CFLAGS)	\
	$(NULL)

libappearance_la_SOURCES= \
	appearance-main.c	\
	appearance-main.h	\
	appearance-desktop.h	\
	appearance-desktop.c	\
	appearance-theme.c	\
	appearance-theme.h	\
	ukui-wp-item.c	\
	ukui-wp-item.h	\
	ukui-wp-xml.c	\
	ukui-wp-xml.h	\
	ukui-wp-info.c  \
	ukui-wp-info.h  \
	appearance.h    \
	appearance-font.c \
	appearance-font.h	\
	$(NULL)

libappearance_la_LIBADD =	\
	@PACKAGE_LIBS@	\
	$(GLIB_LIBS)	\
	$(XFT_LIBS)	\
	$(LIBXML_LIBS)	\
	$(MATE_DESKTOP_LIBS)	\
	$(X11_LIBS)	\
	-lcrypt	\
	$(NULL)

libappearance_la_LDFLAGS = -export-dynamic @PACKAGE_LDFLAGS@
uidir = $(pkgdatadir)/ui
ui_DATA = font_render.ui
-include $(top_srcdir)/git.mk

clean-local :
	rm -f *~ Makefile.in Makefile
