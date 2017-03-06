cappletname=time_data
noinst_LTLIBRARIES=libtime.la

AUTOMAKE_OPTIONS = foreign
#INCLUDES= `pkg-config --cflags gtk+-2.0 glib-2.0`
#LIBS = `pkg-config --libs gtk+-2.0 glib-2.0`

AM_CPPFLAGS =	\
	$(GTK_CFLAGS)	\
	$(GLIB_CFLAGS)	\
	$(NULL)

libtime_la_LIBADD =	\
	$(GTK_LIBS)	\
	$(GLIB_LIBS)	\
	$(NULL)

libtime_la_SOURCES= \
	spy-time.c \
	spt-time.h

-include $(top_srcdir)/git.mk

clean-local :
	rm -f *~ Makefile.in Makefile
