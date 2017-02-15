# This is used in UKUICC_CAPPLETS_CFLAGS
cappletname = keyboard

bin_PROGRAMS = ukui-keyboard-properties

ukui_keyboard_properties_SOURCES = \
	ukui-keyboard-properties.c \
	ukui-keyboard-properties-a11y.c \
	ukui-keyboard-properties-a11y.h \
	ukui-keyboard-properties-xkb.c \
	ukui-keyboard-properties-xkbmc.c \
	ukui-keyboard-properties-xkblt.c \
	ukui-keyboard-properties-xkbltadd.c \
	ukui-keyboard-properties-xkbot.c \
	ukui-keyboard-properties-xkbpv.c \
	ukui-keyboard-properties-xkb.h

ukui_keyboard_properties_LDADD = $(UKUICC_CAPPLETS_LIBS) $(LIBMATEKBDUI_LIBS)

@INTLTOOL_DESKTOP_RULE@

uidir   = $(pkgdatadir)/ui
dist_ui_DATA = ukui-keyboard-properties-a11y-notifications.ui \
               ukui-keyboard-properties-dialog.ui \
               ukui-keyboard-properties-layout-chooser.ui \
               ukui-keyboard-properties-model-chooser.ui \
               ukui-keyboard-properties-options-dialog.ui

desktopdir = $(datadir)/applications
Desktop_in_files = ukui-keyboard.desktop.in
desktop_DATA = $(Desktop_in_files:.desktop.in=.desktop)

AM_CPPFLAGS = \
	$(UKUICC_CAPPLETS_CFLAGS) \
	$(LIBUKUIKBDUI_CFLAGS) \
	-DUKUILOCALEDIR="\"$(datadir)/locale\"" \
	-DUKUICC_DATA_DIR="\"$(pkgdatadir)\"" \
	-DUKUICC_UI_DIR="\"$(uidir)\""
CLEANFILES = $(UKUICC_CAPPLETS_CLEANFILES) $(Desktop_in_files) $(desktop_DATA)
EXTRA_DIST = $(ui_DATA)

-include $(top_srcdir)/git.mk
