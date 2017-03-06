#!/bin/sh
# Run this to generate all the initial makefiles, etc.


srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

if [ "$1"x = "clean"x ]; then
  rm -rf config.* aclocal.m4 compile configure Makefile.in install-sh missing autom4te.cache config.h.in ltmain.sh depcomp m4 config.h.in~ INSTALL Makefile stamp-h1
  exit 0
fi

PKG_NAME="ukui-control-center"

(test -f $srcdir/configure.ac \
  && test -f $srcdir/autogen.sh \
  && test -d $srcdir/shell) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level $PKG_NAME directory"
    exit 1
}

DIE=0

rm -f .using-gnome-libs-package

if ! which mate-autogen.sh ; then
  echo "You need to install the mate-common module and make"
  echo "sure the gnome-autogen.sh script is in your \$PATH."
  exit 1
fi

. mate-autogen.sh

