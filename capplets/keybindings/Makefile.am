# This is used in UKUICC_CAPPLETS_CFLAGS
cappletname = keybinding

bin_PROGRAMS = ukui-keybinding-properties

ukui_keybinding_properties_LDADD = $(UKUICC_CAPPLETS_LIBS)
ukui_keybinding_properties_SOURCES = \
	ukui-keybinding-properties.c \
	eggcellrendererkeys.c \
	eggcellrendererkeys.h \
	eggaccelerators.c \
	eggaccelerators.h

@INTLTOOL_DESKTOP_RULE@

uidir = $(pkgdatadir)/ui
ui_DATA = ukui-keybinding-properties.ui

desktopdir = $(datadir)/applications
Desktop_in_files = ukui-keybinding.desktop.in
desktop_DATA = $(Desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_XML_NOMERGE_RULE@

xmldir = $(pkgdatadir)/keybindings
xml_in_files = 00-multimedia-key.xml.in 01-desktop-key.xml.in
xml_DATA = $(xml_in_files:.xml.in=.xml)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ukui-keybindings.pc

AM_CPPFLAGS = \
	$(UKUICC_CAPPLETS_CFLAGS) \
	-DUKUILOCALEDIR="\"$(datadir)/locale\"" \
	-DUKUICC_DATA_DIR="\"$(pkgdatadir)\"" \
	-DUKUICC_UI_DIR="\"$(uidir)\"" \
	-DUKUICC_KEYBINDINGS_DIR="\"$(pkgdatadir)/keybindings\""
CLEANFILES = \
	$(UKUICC_CAPPLETS_CLEANFILES) \
	$(Desktop_in_files) \
	$(desktop_DATA) \
	$(xml_DATA)
EXTRA_DIST = $(ui_DATA) $(xml_in_files) ukui-keybindings.pc.in


-include $(top_srcdir)/git.mk
