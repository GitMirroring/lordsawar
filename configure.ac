dnl Process this file with autoconf to produce a configure script.
dnl This is useful to force using autoreconf 2.5.x
dnl Copyright (C) 2015. 2016, 2020 Ben Asselstine
dnl This file is licensed under the terms of the GNU GPL version 3 or later.
AC_PREREQ(2.59)
AC_INIT([lordsawar], [0.3.3-pre3], [https://savannah.nongnu.org/bugs/?group=lordsawar], [lordsawar])
AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER(config.h)
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE

IT_PROG_INTLTOOL

GETTEXT_PACKAGE=lordsawar
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"],
                   [The domain to use with gettext])
AM_GLIB_GNU_GETTEXT

PROGRAMNAME_LOCALEDIR=[${datadir}/locale]
AC_SUBST(PROGRAMNAME_LOCALEDIR)
    
dnl Checks for programs.
AC_PROG_CXX
if test -n "$CXX"; then
  gppver=`$CXX --version 2>/dev/null | sed -e '2,$d' -e 's@^\([[^0-9.]]*\) \([[0-9.]]*\)\(.*\)@\2@'`
else
  AC_MSG_ERROR([*** C++ compiler not found.])
fi
AC_SUBST([gppver])

AC_PROG_LIBTOOL

dnl check for libxml and libxslt
PKG_CHECK_MODULES(XSLT, [libxslt >= 1.1.20])
AC_SUBST(XSLT_CFLAGS)

dnl Check for glib

PKG_CHECK_MODULES(GLIBMM, glibmm-2.4)

AC_SUBST(GLIBMM_CFLAGS)
AC_SUBST(GLIBMM_LIBS)

dnl Check for gtkmm

PKG_CHECK_MODULES(GTKMM, gtkmm-3.0)

AC_SUBST(GTKMM_CFLAGS)
AC_SUBST(GTKMM_LIBS)
			     		      
PKG_CHECK_MODULES(XMLPP, libxml++-2.6)
AC_SUBST(XMLPP_CFLAGS)
AC_SUBST(XMLPP_LIBS)

PKG_CHECK_MODULES(ARCHIVE, [libarchive >= 0.28])
AC_SUBST(ARCHIVE_CFLAGS)
AC_SUBST(ARCHIVE_CFLAGS)

dnl Finally, check for libsigc++
PKG_CHECK_MODULES(LIBSIGC, sigc++-2.0)

AC_SUBST(LIBSIGC_CFLAGS)
AC_SUBST(LIBSIGC_LIBS)
AC_SUBST(HOME)

AC_CHECK_LIB(pthread, pthread_create)

dnl----------------------------------------------------------------------
dnl Below here are options to modify default behaviour

dnl Do we want to compile the game list server?
AC_ARG_ENABLE(gls,
            [  --enable-gls     compile game list server (default=yes)],
            use_gls="$enableval", use_gls="yes")
AM_CONDITIONAL([WANT_GLS], [test $use_gls = "yes"])

dnl Do we want to compile the game host server?
AC_ARG_ENABLE(ghs,
            [  --enable-ghs     compile game host server (default=yes)],
            use_ghs="$enableval", use_ghs="yes")
AM_CONDITIONAL([WANT_GHS], [test $use_ghs = "yes"])

use_sound=1
AC_ARG_ENABLE(sound, [  --enable-sound         Enable or disable sound usage (default on)],
                use_sound="$enableval", use_sound="yes")

if test $use_sound = yes; then
PKG_CHECK_MODULES(GSTREAMER, gstreamermm-1.0)
AC_SUBST(GSTREAMER_CFLAGS)
AC_SUBST(GSTREAMER_LIBS)
AC_DEFINE([LW_SOUND], 1, [enables sound])
fi

dnl ...the standard screen resolution...
AC_ARG_WITH(resolution,
           [  --with-resolution=RES standard resolution for lordsawar [default=800x600]], 
            SCREEN_RESOLUTION="$withval", SCREEN_RESOLUTION="800x600")
AC_SUBST(SCREEN_RESOLUTION)

dnl ..do we want to disable optimizations? (useful only for some debugging cases)...
AC_ARG_ENABLE(optimizations,
             [  --enable-optimizations    enable compiler optimizations (disable for debugging) [default=yes]],
             use_opts="$enableval", use_opts="yes")

if test x$use_opts = xno; then
    AC_MSG_WARN([Optimizations have been disabled])
    CXXFLAGS=-g
fi

dnl ...and finally a workaround to get the absolute data path
if test x$prefix = xNONE; then
    prefix="/usr/local"
fi

rm -f src/main.o

eval LORDSAWAR_DATADIR="$datadir/lordsawar"
AC_SUBST(LORDSAWAR_DATADIR)

eval eval DATADIR="$datadir"
AC_SUBST(DATADIR)

echo "$ac_cv_host" | grep "mingw" --quiet
if test "x$?" = "x0"; then
        wincheck=true
else
        wincheck=false
fi
AM_CONDITIONAL([WINDOWS_OS], [test x$wincheck = xtrue])

AC_CONFIG_FILES([make-windows-zip.sh],[chmod +x make-windows-zip.sh])

AC_CONFIG_FILES([
        dat/Makefile 
        dat/icons/Makefile 
        dat/icons/16x16/Makefile 
        dat/icons/22x22/Makefile 
        dat/icons/24x24/Makefile 
        dat/icons/32x32/Makefile 
        dat/icons/48x48/Makefile 
        dat/icons/64x64/Makefile 
        dat/lordsawar.desktop.in
        dat/army/Makefile 
        dat/army/default/Makefile 
        dat/army/bigdefault/Makefile 
        dat/army/duel/Makefile 
        dat/tilesets/Makefile
        dat/tilesets/default/Makefile
        dat/tilesets/bigdefault/Makefile
        dat/tilesets/stark/Makefile
        dat/tilesets/bigstark/Makefile
        dat/tilesets/wesnoth/Makefile
        dat/tilesets/bigwesnoth/Makefile
        dat/shield/Makefile 
        dat/shield/default/Makefile 
        dat/shield/dol/Makefile 
        dat/shield/plain/Makefile 
        dat/citysets/Makefile 
        dat/citysets/default/Makefile 
        dat/citysets/bigdefault/Makefile 
        dat/citysets/dol/Makefile 
        dat/citysets/bigdol/Makefile 
        dat/various/Makefile 
        dat/various/items/Makefile
        dat/various/editor/Makefile
        dat/various/xslt/Makefile
        dat/map/Makefile 
        dat/map/dol/Makefile 
        dat/map/duel/Makefile 
        dat/music/Makefile
        dat/glade/Makefile
        dat/glade/editor/Makefile
        po/Makefile.in
        Makefile 
        src/Makefile
        doc/Makefile
        help/Makefile
        help/figures/Makefile
        src/editor/Makefile
        src/gls/Makefile
        src/ghs/Makefile
        src/utils/Makefile
        src/gui/Makefile])
AC_OUTPUT
echo ""
echo "LordsAWar! is now configured. Type 'make' to compile it."
echo ""
echo "Configuration:"
echo " Installation location : $prefix"
echo " Game-list Server      : $use_gls"
echo " Game-host Server      : $use_ghs"
echo " Enable Sounds         : $use_sound"
echo ""
echo "Extra Options:"
echo " Compiler Optimizations: $use_opts"
