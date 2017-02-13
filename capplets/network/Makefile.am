# This is used in UKUICC_CAPPLETS_CFLAGS
cappletname = network

bin_PROGRAMS = ukui-network-properties

ukui_network_properties_SOURCES = ukui-network-properties.c
ukui_network_properties_LDADD = $(UKUICC_CAPPLETS_LIBS)

@INTLTOOL_DESKTOP_RULE@

uidir = $(pkgdatadir)/ui
dist_ui_DATA = ukui-network-properties.ui

icons16dir = $(datadir)/icons/hicolor/16x16/apps
dist_icons16_DATA = icons/16x16/ukui-network-properties.png
icons22dir = $(datadir)/icons/hicolor/22x22/apps
dist_icons22_DATA = icons/22x22/ukui-network-properties.png
icons24dir = $(datadir)/icons/hicolor/24x24/apps
dist_icons24_DATA = icons/24x24/ukui-network-properties.png
icons32dir = $(datadir)/icons/hicolor/32x32/apps
dist_icons32_DATA = icons/32x32/ukui-network-properties.png
icons48dir = $(datadir)/icons/hicolor/48x48/apps
dist_icons48_DATA = icons/48x48/ukui-network-properties.png
iconssvgdir = $(datadir)/icons/hicolor/scalable/apps
dist_iconssvg_DATA = icons/scalable/ukui-network-properties.svg

desktopdir = $(datadir)/applications
desktop_in_files = ukui-network-properties.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

AM_CPPFLAGS = \
	$(UKUICC_CAPPLETS_CFLAGS) \
	-DUKUILOCALEDIR="\"$(datadir)/locale\"" \
	-DUKUICC_UI_DIR="\"$(uidir)\""

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor
uninstall-hook: update-icon-cache
install-data-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

CLEANFILES = $(UKUICC_CAPPLETS_CLEANFILES) $(desktop_in_files) $(desktop_DATA)

-include $(top_srcdir)/git.mk
