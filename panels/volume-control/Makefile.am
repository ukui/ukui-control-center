cappletname = volumecontrol
noinst_LTLIBRARIES = libvolume.la

SUBDIRS = \
	icons	\
	$(NULL)

libvolume_la_SOURCES = \
	dialog-main.c	\
	dialog-main.h	\
	gvc-balance-bar.c	\
	gvc-balance-bar.h	\
	gvc-channel-bar.c	\
	gvc-channel-bar.h	\
	gvc-channel-map.c	\
	gvc-channel-map.h	\
	gvc-combo-box.c	\
	gvc-combo-box.h	\
	gvc-level-bar.c	\
	gvc-level-bar.h	\
	gvc-log.c	\
	gvc-log.h	\
	gvc-mixer-card.c	\
	gvc-mixer-card.h	\
	gvc-mixer-control.c	\
	gvc-mixer-control.h	\
	gvc-mixer-dialog.c	\
	gvc-mixer-dialog.h	\
	gvc-mixer-event-role.c	\
	gvc-mixer-event-role.h	\
	gvc-mixer-sink.c	\
	gvc-mixer-sink.h	\
	gvc-mixer-sink-input.c	\
	gvc-mixer-sink-input.h	\
	gvc-mixer-source.c	\
	gvc-mixer-source.h	\
	gvc-mixer-source-output.c	\
	gvc-mixer-source-output.h	\
	gvc-mixer-stream.c	\
	gvc-mixer-stream.h	\
	gvc-sound-theme-chooser.c	\
	gvc-sound-theme-chooser.h	\
	gvc-speaker-test.c	\
	gvc-speaker-test.h	\
	sound-theme-file-utils.c	\
	sound-theme-file-utils.h	\
	$(NULL)

AUTOMAKE_OPTIONS = foreign

#INCLUDES = `pkg-config --cflags gtk+-2.0 glib-2.0 unique-1.0 libcanberra-gtk libpulse gdk-2.0 gio-2.0 libxml-2.0 libpulse-mainloop-glib`
#LIBS = `pkg-config --libs gtk+-2.0 glib-2.0 unique-1.0 libcanberra-gtk libpulse gdk-2.0 gio-2.0 libxml-2.0 libpulse-mainloop-glib`
AM_CPPFLAGS = \
	-DSOUND_SET_DIR="\"/usr/share/mate-media/sounds\""	\
	$(GTK_CFLAGS)	\
	$(GLIB_CFLAGS)	\
	$(UNIQUE_CFLAGS)	\
	$(CANBERRA_CFLAGS)	\
	$(PULSE_CFLAGS)	\
	$(GDK_CFLAGS)	\
	$(LIBXML_CFLAGS)	\
	$(NULL)

libvolume_la_LIBADD =	\
	$(GTK_LIBS)	\
	$(GLIB_LIBS)	\
	$(UNIQUE_LIBS)	\
	$(CANBERRA_LIBS)	\
	$(PULSE_LIBS)	\
	$(GDK_LIBS)	\
	$(LIBXML_LIBS)	\
	$(NULL)

clean-local : 
	rm -f *~ Makefile.in Makefile
