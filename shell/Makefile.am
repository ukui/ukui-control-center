AM_CPPFLAGS = 						\
        -DUIDIR="\"$(uidir)\""				\
	-I$(top_builddir)/panels/time-data		\
	-I$(top_builddir)/panels/user-accounts		\
	-I$(top_builddir)/panels/appearance		\
	-I$(top_builddir)/panels/default-app		\
	-I$(top_builddir)/panels/keyboard		\
	-I$(top_builddir)/panels/mouse			\
	-I$(top_builddir)/panels/network-proxy		\
	-I$(top_builddir)/panels/session-properties	\
	-I$(top_builddir)/panels/display		\
	-I$(top_builddir)/panels/power			\
	-I$(top_builddir)/panels/volume-control	\
	$(UNIQUE_CFLAGS)	\
	@PACKAGE_CFLAGS@

bin_PROGRAMS = ukui-control-center

ukui_control_center_SOURCES = 		\
	 ukui-control-center.c		\
	 mainwindow.c			\
	 mainwindow.h


ukui_control_center_LDADD = 							\
	@PACKAGE_LIBS@								\
	$(UNIQUE_LIBS)	\
	$(top_builddir)/panels/time-data/libtime.la				\
	$(top_builddir)/panels/user-accounts/libuser-accounts.la 		\
	$(top_builddir)/panels/appearance/libappearance.la 		\
	$(top_builddir)/panels/default-app/libdefaultapp.la  			\
	$(top_builddir)/panels/keyboard/libkeyboard.la  			\
	$(top_builddir)/panels/mouse/libmouseapp.la  				\
	$(top_builddir)/panels/network-proxy/libnetworkproxy.la  		\
	$(top_builddir)/panels/session-properties/libsessionproperties.la	\
	$(top_builddir)/panels/display/libdisplayapp.la  			\
	$(top_builddir)/panels/power/libpower.la 				\
	$(top_builddir)/panels/volume-control/libvolume.la	\
	-lm

ukui_control_center_LDFLAGS = -export-dynamic @PACKAGE_LDFLAGS@ 

uidir = $(pkgdatadir)/ui
ui_DATA = shell.ui

desktopdir=$(datadir)/applications
desktop_DATA=ukui-control-center.desktop

EXTRA_DIST =                                    \
        $(ui_DATA)                              

clean-local :
	rm -f *~ Makefile.in Makefile
