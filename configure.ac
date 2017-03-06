AC_INIT([ukui-control-center], [1.0], [penghuan@ukuios.cn])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([shell])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.9 no-dist-gzip dist-xz tar-ustar])
AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes]]))

AC_PROG_CC
AM_PROG_CC_C_O
AC_STDC_HEADERS
AM_PROG_LIBTOOL

# Use the Yelp documentation framework
YELP_HELP_INIT

AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

# Internationalization support

IT_PROG_INTLTOOL([0.37.1])

GETTEXT_PACKAGE=ukui-control-center
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])

AH_TEMPLATE([GETTEXT_PACKAGE])
AH_TEMPLATE([HAVE_CATGETS])
AH_TEMPLATE([HAVE_GETTEXT])
AH_TEMPLATE([HAVE_LC_MESSAGES])

AM_GLIB_GNU_GETTEXT
GLIB_DEFINE_LOCALEDIR(MATELOCALEDIR)

dnl ---------------------------------------------------------------------------
dnl - Make paths available for source files
dnl ---------------------------------------------------------------------------
AC_SUBST(SYSCONFDIR, $sysconfdir)
AC_SUBST(DATADIR, $datadir)
AC_SUBST(BINDIR, $bindir)
AC_SUBST(SBINDIR, $sbindir)

dnl ==============================================
dnl Special GSettings section
dnl ==============================================

GLIB_GSETTINGS

dnl ----------------------------------------------------------------------------
dnl - Library dependencies
dnl ----------------------------------------------------------------------------
pkg-config=2.13.0
GIO_REQUIRED=2.25.0
GTK_REQUIRED=3.0.0
GDK_REQUIRED=3.0.0
GNOME_KEYRING_REQUIRED=3.0.0
DBUS_REQUIRED=1.0
DBUS_GLIB_REQUIRED=0.70
LIBNOTIFY_REQUIRED=0.7.0
CAIRO_REQUIRED=1.0.0
UNIQUE_REQUIRED=0.9.4
LIBPANEL_REQUIRED=1.5.0
XRANDR_REQUIRED=1.2.0
CANBERRA_REQUIRED=0.10
UPOWER_REQUIRED=0.9.1


PKG_CHECK_MODULES(GTK, [gtk+-3.0 >= 3.0.0])
#PKG_CHECK_MODULES(GTK_PIXBUF, [gdk-pixbuf-3.0])
dnl ----------------------------------------------------------------------------
dnl GTK library version
dnl ----------------------------------------------------------------------------

AC_MSG_CHECKING([which gtk+ version to compile against])
AC_ARG_WITH([gtk],
        [AS_HELP_STRING([--with-gtk=2.0|3.0],[which gtk+ version to compile against (default: 2.0)])],
        [case "$with_gtk" in
                2.0|3.0) ;;
                *) AC_MSG_ERROR([invalid gtk version specified]) ;;
         esac],
        [with_gtk=3.0])
AC_MSG_RESULT([-----------------------------------------------------])
AC_MSG_RESULT([$with_gtk])
AC_MSG_RESULT([-----------------------------------------------------])

case "$with_gtk" in
        2.0) GTK_API_VERSION=2.0
                GTK_REQUIRED=2.17.7
                CANBERRA_GTK=libcanberra-gtk
                ;;
        3.0) GTK_API_VERSION=3.0
                GTK_REQUIRED=3.0.0
                CANBERRA_GTK=libcanberra-gtk3
                ;;
esac


PACKAGE_CFLAGS="-g -Wall $GTK_CFLAGS"
PACKAGE_LIBS="-g $GTK_LIBS"

AC_SUBST([PACKAGE_CFLAGS])
AC_SUBST([PACKAGE_LIBS])
AC_SUBST([PACKAGE_LDFLAGS])

PKG_CHECK_MODULES(CANBERRA, libcanberra-gtk3)
PKG_CHECK_MODULES(PULSE, libpulse libpulse-mainloop-glib)

#mate-settings-daemon
PKG_CHECK_MODULES(MATE_SD, [mate-settings-daemon])

#upower
PKG_CHECK_MODULES(UPOWER, [upower-glib >= 0.9.1])

#upower
PKG_CHECK_MODULES(OOBS, [liboobs-1 >= 2.91.1])

#mate-desktop
PKG_CHECK_MODULES(MATE_DESKTOP, [mate-desktop-2.0])
PKG_CHECK_MODULES(MATE_KBD, [libmatekbd])
PKG_CHECK_MODULES(MATE_KBD_UI, [libmatekbdui])

#xft
PKG_CHECK_MODULES(XFT, [xft])

#libxml
PKG_CHECK_MODULES(LIBXML,[libxml-2.0])

PKG_CHECK_MODULES(DCONF, [dconf])
dnl ---------------------------------------------------------------------------
dnl - Check library dependencies
dnl ---------------------------------------------------------------------------
PKG_CHECK_MODULES(GLIB, glib-2.0 >= $GLIB_REQUIRED gobject-2.0 gio-2.0 >= $GIO_REQUIRED gio-unix-2.0)

PKG_CHECK_MODULES(CANBERRA, $CANBERRA_GTK >= $CANBERRA_REQUIRED)

PKG_CHECK_MODULES(DBUS,[
 dbus-glib-1 >= $DBUS_GLIB_REQUIRED
 dbus-1 >= $DBUS_REQUIRED
 gthread-2.0])

PKG_CHECK_MODULES(XI, xi)

PKG_CHECK_MODULES(MATE, [
 gtk+-$GTK_API_VERSION >= $GTK_REQUIRED
 cairo >= $CAIRO_REQUIRED])

PKG_CHECK_MODULES(GDK, [
 gdk-$GTK_API_VERSION >= $GDK_REQUIRED
 gdk-x11-$GTK_API_VERSION >= $GDK_REQUIRED])

PKG_CHECK_MODULES(X11, [
 xrandr >= $XRANDR_REQUIRED
 xrender x11 xext xproto])

PKG_CHECK_MODULES(X11, x11 xrender)

PKG_CHECK_EXISTS(
 [xrandr >= 1.3],
 [AC_DEFINE(HAVE_XRANDR_13, 1, [xrandr 1.3 available])])

PKG_CHECK_MODULES(LIBNOTIFY, libnotify >= $LIBNOTIFY_REQUIRED)


dnl ----------------------------------------------------------------------------
dnl - Unique
dnl ----------------------------------------------------------------------------
#PKG_CHECK_MODULES(UNIQUE, unique-1.0 >= $UNIQUE_REQUIRED)
case "$with_gtk" in
        2.0) LIBUNIQUE_VERSION=1.0
                ;;
        3.0) LIBUNIQUE_VERSION=3.0
                ;;
esac

PKG_CHECK_MODULES(UNIQUE, unique-$LIBUNIQUE_VERSION, enable_libunique=yes, enable_libunique=no)

AC_SUBST([UNIQUE_CFLAGS])
AC_SUBST([UNIQUE_LIBS])

AC_ARG_ENABLE(unique, [  --enable-unique         enable the use of libunique instead of g_application or gtk_application], enable_libunique=yes,)

if test "x$enable_libunique" = "xyes"; then
        UNIQUE_CFLAGS="$UNIQUE_CFLAGS -DUSE_UNIQUE=1"
        # fix for deprecated on 2.26
        # unique en la version 1.0, contiene simbolos no definidos a partir de
        # glib 2.26, pero puede ser obviado si se utiliza -DG_CONST_RETURN=const
        if $PKG_CONFIG --atleast-version 2.26 glib-2.0; then
                UNIQUE_CFLAGS="$UNIQUE_CFLAGS -DG_CONST_RETURN=const"
        fi
fi


dnl =======================================
dnl Panels
dnl =======================================

PANELS_DIR="${libdir}/ukui-control-center/panels"
AC_SUBST(PANELS_DIR)

PANEL_CFLAGS="-I\$(top_srcdir)/ -DG_LOG_DOMAIN=\"\\\"ukui-control-center\\\"\""
AC_SUBST(PANEL_CFLAGS)

#PANEL_LDFLAGS="-export_dynamic -avoid-version -module -no-undefined -export-symbols-regex '^g_io_module_(load|unload)'"
#AC_SUBST(PANEL_LDFLAGS)

AC_OUTPUT([
Makefile
po/Makefile.in
data/Makefile
data/color/Makefile
shell/Makefile
panels/Makefile
panels/time-data/Makefile
panels/appearance/Makefile
panels/user-accounts/Makefile
panels/default-app/Makefile
panels/keyboard/Makefile
panels/mouse/Makefile
panels/network-proxy/Makefile
panels/session-properties/Makefile
panels/display/Makefile
panels/power/Makefile
panels/volume-control/Makefile
panels/volume-control/icons/Makefile
panels/volume-control/icons/16x16/Makefile
panels/volume-control/icons/16x16/status/Makefile
panels/volume-control/icons/22x22/Makefile
panels/volume-control/icons/22x22/status/Makefile
panels/volume-control/icons/24x24/Makefile
panels/volume-control/icons/24x24/status/Makefile
panels/volume-control/icons/32x32/Makefile
panels/volume-control/icons/32x32/status/Makefile
panels/volume-control/icons/scalable/Makefile
panels/volume-control/icons/scalable/devices/Makefile
panels/volume-control/icons/scalable/status/Makefile
])

