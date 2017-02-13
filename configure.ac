AC_INIT([ukui-control-center], [1.16.0], [htts://www.maete-desktop.org/])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([shell])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.9 no-dist-gzip dist-xz tar-ustar])
AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_PROG_CC
AM_PROG_CC_C_O
AC_STDC_HEADERS
AM_PROG_LIBTOOL

# Use the Yelp documentation framework
YELP_HELP_INIT

changequote(,)dnl
if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi
changequote([,])dnl

# Internationalization support

IT_PROG_INTLTOOL([0.50.1])

GETTEXT_PACKAGE=ukui-control-center
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])

AM_GLIB_GNU_GETTEXT

UKUI_COMMON_INIT
UKUI_DEBUG_CHECK
UKUI_COMPILE_WARNINGS([maximum])
UKUI_MAINTAINER_MODE_DEFINES

AC_PATH_XTRA
x_libs="$X_PRE_LIBS $X_LIBS -lX11 $X_EXTRA_LIBS"

AC_CHECK_FUNC(XScreenSaverQueryInfo, [
  TYPING_BREAK="typing-break"], [
  save_LIBS=$LIBS
  LIBS="$LIBS $X_PRE_LIBS $X_LIBS -lXss -lXext -lX11"
  AC_CHECK_LIB(Xss, XScreenSaverQueryInfo, [
    TYPING_BREAK="typing-break"
    SCREENSAVER_LIBS="$X_PRE_LIBS $X_LIBS -lXss -lXext -lX11"], [
    LIBS=$save_LIBS
    LIBS="$LIBS $X_PRE_LIBS $X_LIBS -lXext -lX11"
    AC_CHECK_LIB(Xext, XScreenSaverQueryInfo, [
      TYPING_BREAK="typing-break"
      SCREENSAVER_LIBS="$X_PRE_LIBS $X_LIBS -lXext -lX11"],[])])
  LIBS=$save_LIBS])
AC_SUBST(TYPING_BREAK)
AC_SUBST(SCREENSAVER_LIBS)

dnl ==============================================
dnl Check GTK+ API version
dnl ==============================================

APPINDICATOR_REQUIRED=0.0.13

AC_MSG_CHECKING([which gtk+ version to compile against])
AC_ARG_WITH([gtk],
  [AS_HELP_STRING([--with-gtk=2.0|3.0],[which gtk+ version to compile against (default: 2.0)])],
  [case "$with_gtk" in
     2.0|3.0) ;;
     *) AC_MSG_ERROR([invalid gtk version specified]) ;;
   esac],
  [with_gtk=2.0])
AC_MSG_RESULT([$with_gtk])

case "$with_gtk" in
  2.0) GTK_API_VERSION=2.0
       GTK_REQUIRED=2.24.0
       UKUI_DESKTOP_REQUIRED=1.15.1
       LIBCANBERRA_API_VERSION=
       LIBUNIQUE_API_VERSION=1.0
       ENGINES_FOLDER="engines"
       APPINDICATOR_API_VERSION=
       ;;
  3.0) GTK_API_VERSION=3.0
       GTK_REQUIRED=3.0.0
       UKUI_DESKTOP_REQUIRED=1.15.1
       LIBCANBERRA_API_VERSION=3
       LIBUNIQUE_API_VERSION=3.0
       ENGINES_FOLDER="theming-engines"
       APPINDICATOR_API_VERSION=3
       ;;
esac
AC_SUBST(GTK_API_VERSION)

PKG_CHECK_MODULES(LIBCANBERRA_GTK, libcanberra-gtk$LIBCANBERRA_API_VERSION, [
    CANBERRA_GTK=1
    AC_SUBST(LIBCANBERRA_GTK_CFLAGS)
    AC_SUBST(LIBCANBERRA_GTK_LIBS)
    ], [:])
AM_CONDITIONAL(HAVE_LIBCANBERRA_GTK, test "x$CANBERRA_GTK" = "x1")

AM_CONDITIONAL(HAVE_TYPING_BREAK, test "x$TYPING_BREAK" = xtyping-break)

dnl app indicator
APPINDICATOR_PKG=appindicator$APPINDICATOR_API_VERSION-0.1

AC_ARG_ENABLE(appindicator,
                        AS_HELP_STRING([--enable-appindicator[=@<:@no/auto/yes@:>@]],[Build support for application indicators ]),
                        [enable_appindicator=$enableval],
                        [enable_appindicator="auto"])

if test x$enable_appindicator = xauto ; then
        PKG_CHECK_EXISTS([$APPINDICATOR_PKG >= $APPINDICATOR_REQUIRED],
                         enable_appindicator="yes",
                         enable_appindicator="no")
fi

if test x$enable_appindicator = xyes ; then
        PKG_CHECK_EXISTS([$APPINDICATOR_PKG >= $APPINDICATOR_REQUIRED],,
                         AC_MSG_ERROR([appindicator-0.1 is not installed]))
        PKG_CHECK_MODULES(APP_INDICATOR,
                        $APPINDICATOR_PKG >= $APPINDICATOR_REQUIRED)
        AC_SUBST(APP_INDICATOR_CFLAGS)
        AC_SUBST(APP_INDICATOR_LIBS)
        AC_DEFINE(HAVE_APP_INDICATOR, 1, [Have AppIndicator])
fi
AM_CONDITIONAL(HAVE_APP_INDICATOR, test x"$enable_appindicator" = xyes)

dnl keyboard-properties-capplet
savecppflags=$CPPFLAGS
CPPFLAGS="$CPPFLAGS $X_CFLAGS"
AC_CHECK_HEADERS([X11/Xlib.h])
AC_CHECK_LIB(Xxf86misc, XF86MiscQueryExtension, [
  AC_CHECK_HEADERS([X11/extensions/xf86misc.h], [XF86MISC_LIBS="-lXxf86misc"],[],
[#if HAVE_X11_XLIB_H
#include <X11/Xlib.h>
#endif
])])
AC_SUBST(XF86MISC_LIBS)
AC_CHECK_HEADERS(X11/extensions/XKB.h)
CPPFLAGS=$savecppflags

AC_CHECK_LIB(m, floor)

dnl ==============================================
dnl Check that we meet the  dependencies
dnl ==============================================

PKG_CHECK_MODULES(GMODULE,gmodule-2.0,[GMODULE_ADD="gmodule-2.0"],[GMODULE_ADD=""])
COMMON_MODULES="gtk+-$GTK_API_VERSION >= $GTK_REQUIRED dnl
 $GMODULE_ADD dnl
 glib-2.0 >= 2.36 dnl
 gthread-2.0 dnl
 gio-2.0 dnl
 gio-unix-2.0 dnl
 libxml-2.0 dnl
 ukui-desktop-2.0 >= $UKUI_DESKTOP_REQUIRED"
PKG_CHECK_MODULES(CAPPLET, $COMMON_MODULES)
PKG_CHECK_MODULES(UKUICC, $COMMON_MODULES libukui-menu >= 1.1.0)
PKG_CHECK_MODULES(UKUICC_SHELL, $COMMON_MODULES libukui-menu unique-$LIBUNIQUE_API_VERSION)
PKG_CHECK_MODULES(DBUS, dbus-1 dbus-glib-1)
PKG_CHECK_MODULES(UKUI_DESKTOP, ukui-desktop-2.0 >= UKUI_DESKTOP_REQUIRED)
PKG_CHECK_MODULES(DEFAULT_APPLICATIONS_CAPPLET, libxml-2.0)
PKG_CHECK_MODULES(UKWM, libukwm-private >= 1.13.1)
PKG_CHECK_MODULES(TYPING, $GMODULE_ADD glib-2.0 > 2.36 gio-2.0 gtk+-$GTK_API_VERSION)
PKG_CHECK_MODULES(GSD_DBUS, ukui-settings-daemon >= 1.13.1)
PKG_CHECK_MODULES(GIO, gio-2.0)

PKG_CHECK_MODULES([DCONF], [dconf >= 0.13.4])
AC_SUBST(DCONF_CFLAGS)
AC_SUBST(DCONF_LIBS)

gtk_lib_dir=`$PKG_CONFIG --variable libdir gtk+-$GTK_API_VERSION`
gtk_binary_version=`$PKG_CONFIG --variable gtk_binary_version gtk+-$GTK_API_VERSION`
GTK_ENGINE_DIR="$gtk_lib_dir/gtk-$GTK_API_VERSION/$gtk_binary_version/$ENGINES_FOLDER"
AC_SUBST(GTK_ENGINE_DIR)

PKG_CHECK_MODULES(GLIB, glib-2.0 $GMODULE_ADD)

dnl
dnl Check dependencies of libukui-slab
dnl
PKG_CHECK_MODULES(LIBSLAB, [
			   gio-2.0 ukui-desktop-2.0 librsvg-2.0 libukui-menu pango
			   ],
			   have_libukuislab=yes,
			   have_libukuislab=no)
WARN_CFLAGS="-Wall"

AC_SUBST(LIBUKUISLAB_CFLAGS)
AC_SUBST(LIBUKUISLAB_LIBS)
AC_SUBST(WARN_CFLAGS)

AM_CONDITIONAL(HAVE_LIBUKUISLAB, [test $have_libukuislab = yes])

PKG_CHECK_MODULES(FONT_CAPPLET, $COMMON_MODULES pango)
PKG_CHECK_MODULES(FONT_VIEWER, $COMMON_MODULES freetype2)

PKG_CHECK_MODULES(AT_CAPPLET, $COMMON_MODULES)

PKG_CHECK_MODULES(DISPLAY_CAPPLET, $COMMON_MODULES)
DISPLAY_CAPPLET_LIBS="$DISPLAY_CAPPLET_LIBS"

CAPPLET_LIBS="$CAPPLET_LIBS $x_libs"
UKUICC_LIBS="$UKUICC_LIBS $x_libs"

dnl
dnl Check for XCursor support.  If it exists, then we compile the
dnl mouse capplet with support for it turned on
dnl
have_xcursor=no
PKG_CHECK_MODULES(XCURSOR, [
	xcursor
	],
	have_xcursor=yes
	AC_DEFINE(HAVE_XCURSOR, 1, [Define if the Xcursor extension is available]),
	have_xcursor=no)
AM_CONDITIONAL(HAVE_XCURSOR, [test $have_xcursor=yes])
AC_SUBST(XCURSOR_CFLAGS)
AC_SUBST(XCURSOR_LIBS)

if test x$have_xcursor = xyes; then
  AC_MSG_CHECKING([for XCURSOR cursor location])
  XCURSOR_ICONDIR=$($PKG_CONFIG --variable=icondir xcursor)
  if test "x$XCURSOR_ICONDIR" != x; then
    AC_DEFINE_UNQUOTED(XCURSOR_ICONDIR, "${XCURSOR_ICONDIR}", [Define to the xcursor icon path])
  else
    XCURSOR_ICONDIR=none
  fi
  AC_MSG_RESULT([${XCURSOR_ICONDIR}])
fi

CAPPLET_LIBS="$CAPPLET_LIBS $XCURSOR_LIBS"

dnl =============================================
dnl X Input library >= 1.2 with property support
dnl =============================================
have_xinput=no
PKG_CHECK_MODULES(XINPUT, [xi >= 1.2],
	have_xinput=yes
	AC_DEFINE(HAVE_XINPUT, 1, [Define if the XInput extension is available]),
	have_xinput=no)
AM_CONDITIONAL(HAVE_XINPUT, [test $have_xinput=yes])
AC_SUBST(XINPUT_CFLAGS)
AC_SUBST(XINPUT_LIBS)

CAPPLET_LIBS="$CAPPLET_LIBS $XINPUT_LIBS"

dnl ==============
dnl gswitchit
dnl ==============
PKG_CHECK_MODULES(LIBUKUIKBD, [libukuikbd >= 1.1.0 libxklavier >= 4.0])
AC_SUBST(LIBUKUIKBD_CFLAGS)
AC_SUBST(LIBUKUIKBD_LIBS)

PKG_CHECK_MODULES(LIBUKUIKBDUI, [libukuikbdui >= 1.1.0])
AC_SUBST(LIBUKUIKBDUI_CFLAGS)
AC_SUBST(LIBUKUIKBDUI_LIBS)

dnl ==============================================
dnl End: Check that we meet the  dependencies
dnl ==============================================

AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal, no)

if test x"$GLIB_GENMARSHAL" = xno; then
  AC_MSG_ERROR([glib-genmarshal executable not found in your path - should be installed with glib])
fi

AC_SUBST(GLIB_GENMARSHAL)

dnl ==============================================
dnl Special GSettings section
dnl ==============================================

GLIB_GSETTINGS

dnl ==============================================
dnl Define the main variables
dnl ==============================================
COMMON_CFLAGS="-I\$(top_srcdir)/capplets/common"
COMMON_LIBS="\$(top_builddir)/capplets/common/libcommon.la"

EXTRA_CFLAGS="-I\$(top_srcdir)/ -DG_LOG_DOMAIN=\"\\\"\$(cappletname)-properties\\\"\""

UKUICC_CAPPLETS_CFLAGS="${COMMON_CFLAGS} ${CAPPLET_CFLAGS} ${EXTRA_CFLAGS} ${DBUS_CFLAGS}"
UKUICC_CAPPLETS_LIBS="${COMMON_LIBS} ${CAPPLET_LIBS} ${DBUS_LIBS}"

UKUICC_CAPPLETS_CLEANFILES="\$(desktop) \$(desktop).in"
UKUICC_CAPPLETS_EXTRA_DIST="ChangeLog \$(desktop).in.in \$(cappletname)-capplet.png \$(pixmaps_DATA)"

AC_SUBST(UKUICC_CAPPLETS_EXTRA_DIST)
AC_SUBST(UKUICC_CAPPLETS_CLEANFILES)
AC_SUBST(UKUICC_CAPPLETS_CFLAGS)
AC_SUBST(UKUICC_CAPPLETS_LIBS)

dnl =======================================
dnl Update Mime Database
dnl =======================================

AC_PATH_PROG(UPDATE_MIME_DATABASE, update-mime-database, no)

AC_ARG_ENABLE(update-mimedb,
   AC_HELP_STRING([--disable-update-mimedb],
                   [do not update mime database after installation]),,
                   enable_update_mimedb=yes)
AM_CONDITIONAL(ENABLE_UPDATE_MIMEDB, test x$enable_update_mimedb = xyes)

AC_CONFIG_FILES([
Makefile
org.ukui.control-center.gschema.xml
org.ukui.control-center.keybinding.gschema.xml
capplets/Makefile
capplets/about-me/Makefile
capplets/about-me/ukui-about-me.desktop.in
capplets/about-me/icons/Makefile
capplets/accessibility/Makefile
capplets/accessibility/at-properties/ukui-at-properties.desktop.in
capplets/accessibility/at-properties/Makefile
capplets/appearance/Makefile
capplets/appearance/data/Makefile
capplets/appearance/data/ukui-appearance-properties.desktop.in
capplets/appearance/data/ukui-theme-installer.desktop.in
capplets/common/Makefile
capplets/default-applications/Makefile
capplets/default-applications/ukui-default-applications-properties.desktop.in
capplets/default-applications/ukui-default-applications.pc
capplets/display/Makefile
capplets/display/ukui-display-properties.desktop.in
capplets/keybindings/Makefile
capplets/keybindings/ukui-keybindings.pc
capplets/keybindings/ukui-keybinding.desktop.in
capplets/keyboard/Makefile
capplets/keyboard/ukui-keyboard.desktop.in
capplets/mouse/Makefile
capplets/mouse/ukui-settings-mouse.desktop.in
capplets/network/Makefile
capplets/network/ukui-network-properties.desktop.in
capplets/windows/Makefile
capplets/windows/ukui-window-properties.desktop.in
font-viewer/Makefile
font-viewer/ukui-font-viewer.desktop.in
help/Makefile
libwindow-settings/Makefile
libwindow-settings/ukui-window-settings-2.0.pc
man/Makefile
po/Makefile.in
libslab/ukui-slab.pc
libslab/Makefile
shell/Makefile
shell/ukuicc.desktop.in
typing-break/Makefile
])

dnl due to a bug in intltool we need to expand something from the root last control-center.spec
AC_OUTPUT

dnl ---------------------------------------------------------------------------
dnl - Show summary
dnl ---------------------------------------------------------------------------

echo "
              ukui-control-center

        GTK+ API version:      ${GTK_API_VERSION}
        Appindicator:          ${enable_appindicator}
        Libukui-slab:         ${have_libukuislab}
        XCursor support:       ${have_xcursor}
        XInput support:        ${have_xinput}
"
