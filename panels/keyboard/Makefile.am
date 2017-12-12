cappletname=keyboard
noinst_LTLIBRARIES=libkeyboard.la

AUTOMAKE_OPTIONS=foreign
#INCLUDES= `pkg-config --cflags gtk+-2.0 gio-2.0 glib-2.0 ukui-settings-daemon libmatekbd gdk-2.0 libmatekbdui x11 dconf gobject-2.0`
#LIBS= `pkg-config --libs gtk+-2.0 gio-2.0 glib-2.0 ukui-settings-daemon libmatekbd gdk-2.0 libmatekbdui x11 dconf gobject-2.0`
AM_CPPFLAGS = \
	-DUIDIR="\"$(uidir)\""	\
	-DUKUI_KEYBINDINGS_DIR="\"$(pkgdatadir)/keybindings\""	\
	$(GTK_CFLAGS)	\
	$(GLIB_CFLAGS)	\
	$(UKUI_SD_CFLAGS)	\
	$(MATE_KBD_CFLAGS)	\
	$(GDK_CFLAGS)	\
	$(MATE_KBD_UI_CFLAGS)	\
	$(X11_CFLAGS)	\
	$(DCONF_CFLAGS)	\
	$(NULL)

libkeyboard_la_LIBADD =	\
	$(GTK_LIBS)	\
	$(GLIB_LIBS)	\
	$(UKUI_SD_LIBS)	\
	$(MATE_KBD_LIBS)	\
	$(GDK_LIBS)	\
	$(MATE_KBD_UI_LIBS)	\
	$(X11_LIBS)	\
	$(DCONF_LIBS)	\
	$(NULL)

libkeyboard_la_SOURCES= \
	mate-keyboard-properties.c	\
	mate-keyboard-properties.h	\
	mate-keyboard-properties-a11y.c	\
	mate-keyboard-properties-a11y.h	\
	mate-keyboard-properties-xkb.c	\
	mate-keyboard-properties-xkblt.c	\
	mate-keyboard-properties-xkbltadd.c	\
	mate-keyboard-properties-xkbmc.c	\
	mate-keyboard-properties-xkbot.c	\
	mate-keyboard-properties-xkbpv.c	\
	mate-keyboard-properties-xkb.h	\
	eggaccelerators.c	\
	eggaccelerators.h	\
	eggcellrendererkeys.c	\
	eggcellrendererkeys.h	\
	mate-keybinding-properties.c	\
	mate-keybinding-properties.h	\
	wm-common.c	\
	wm-common.h	\
	dconf-util.c	\
	dconf-util.h

uidir = $(pkgdatadir)/ui
ui_DATA = mate-keyboard-properties-layout-chooser.ui shortcut_dialog.ui
@INTLTOOL_XML_NOMERGE_RULE@
xmldir = $(pkgdatadir)/keybindings
xml_in_files = 01-desktop-key.xml.in
xml_DATA = $(xml_in_files:.xml.in=.xml)

EXTRA_DIST= $(xml_in_files) $(uidir)
-include $(top_srcdir)/git.mk

clean-local :
	rm -f *~ Makefile.in Makefile 00-multimedia-key.xml 01-desktop-key.xml
