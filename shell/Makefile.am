REAL_LIBSLAB_CFLAGS = -I$(top_srcdir)/libslab
REAL_LIBSLAB_LIBS = $(top_builddir)/libslab/libukui-slab.la

AM_CPPFLAGS =					\
	-I$(top_srcdir)				\
	$(REAL_LIBSLAB_CFLAGS)		\
	$(UKUICC_SHELL_CFLAGS)  	\
	-DUKUILOCALEDIR="\"$(datadir)/locale\""

bin_PROGRAMS = ukui-control-center 

ukui_control_center_SOURCES =		\
	control-center.c

ukui_control_center_LDADD =						\
	$(UKUICC_SHELL_LIBS)						\
	$(REAL_LIBSLAB_LIBS)

sysdir = $(datadir)/applications
sys_in_files = ukuicc.desktop.in
sys_DATA = $(sys_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

directorydir = $(datadir)/desktop-directories
directory_in_files = ukuicc.directory.in
directory_DATA = $(directory_in_files:.directory.in=.directory)
@INTLTOOL_DIRECTORY_RULE@

menudir = $(sysconfdir)/xdg/menus
menu_DATA = ukuicc.menu

EXTRA_DIST = ukuicc.desktop.in.in ukuicc.directory.in ukuicc.menu

DISTCLEANFILES = ukuicc.desktop ukuicc.desktop.in ukuicc.directory

-include $(top_srcdir)/git.mk
