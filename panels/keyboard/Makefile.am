cappletname=keyboard
noinst_LTLIBRARIES=libkeyboard.la

AUTOMAKE_OPTIONS=foreign
#INCLUDES= `pkg-config --cflags gtk+-2.0 gio-2.0 glib-2.0 mate-settings-daemon libmatekbd gdk-2.0 libmatekbdui x11 dconf gobject-2.0`
#LIBS= `pkg-config --libs gtk+-2.0 gio-2.0 glib-2.0 mate-settings-daemon libmatekbd gdk-2.0 libmatekbdui x11 dconf gobject-2.0`
AM_CPPFLAGS = \
	-DUIDIR="\"$(uidir)\""	\
	-DUKUI_KEYBINDINGS_DIR="\"$(pkgdatadir)/keybindings\""	\
	$(GTK_CFLAGS)	\
	$(GLIB_CFLAGS)	\
	$(MATE_SD_CFLAGS)	\
	$(MATE_KBD_CFLAGS)	\
	$(GDK_CFLAGS)	\
	$(MATE_KBD_UI_CFLAGS)	\
	$(X11_CFLAGS)	\
	$(DCONF_CFLAGS)	\
	$(NULL)

libkeyboard_la_LIBADD =	\
	$(GTK_LIBS)	\
	$(GLIB_LIBS)	\
	$(MATE_SD_LIBS)	\
	$(MATE_KBD_LIBS)	\
	$(GDK_LIBS)	\
	$(MATE_KBD_UI_LIBS)	\
	$(X11_LIBS)	\
	$(DCONF_LIBS)	\
	$(NULL)

libkeyboard_la_SOURCES= \
	ukui-keyboard-properties.c	\
	ukui-keyboard-properties.h	\
	ukui-keyboard-properties-a11y.c	\
	ukui-keyboard-properties-a11y.h	\
	ukui-keyboard-properties-xkb.c	\
	ukui-keyboard-properties-xkblt.c	\
	ukui-keyboard-properties-xkbltadd.c	\
	ukui-keyboard-properties-xkbmc.c	\
	ukui-keyboard-properties-xkbot.c	\
	ukui-keyboard-properties-xkbpv.c	\
	ukui-keyboard-properties-xkb.h	\
	eggaccelerators.c	\
	eggaccelerators.h	\
	eggcellrendererkeys.c	\
	eggcellrendererkeys.h	\
	ukui-keybinding-properties.c	\
	ukui-keybinding-properties.h	\
	wm-common.c	\
	wm-common.h	\
	dconf-util.c	\
	dconf-util.h

uidir = $(pkgdatadir)/ui
ui_DATA = ukui-keyboard-properties-layout-chooser.ui shortcut_dialog.ui
@INTLTOOL_XML_NOMERGE_RULE@
xmldir = $(pkgdatadir)/keybindings
xml_in_files = 01-desktop-key.xml.in
xml_DATA = $(xml_in_files:.xml.in=.xml)

EXTRA_DIST= $(xml_in_files) $(uidir)
-include $(top_srcdir)/git.mk

clean-local :
	rm -f *~ Makefile.in Makefile 00-multimedia-key.xml 01-desktop-key.xml
