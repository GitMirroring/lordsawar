dnl Copyright (C) 2015, 2016, 2020, 2026 Ben Asselstine
dnl This file is licensed under the terms of the GNU GPL version 3 or later.

AC_PREREQ([2.69])
AC_INIT([lw], [0.4.0], [https://savannah.nongnu.org/bugs/?group=lordsawar], [lordsawar])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([1.15 foreign subdir-objects])
AM_SILENT_RULES([yes])

IT_PROG_INTLTOOL([0.35.0])

# Check for programs
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Initialize libtool for shared libraries
LT_INIT

# ============================
# GETTEXT / I18N SETUP
# ============================

GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [lw])

# Initialize GNU gettext - this creates po/Makefile.in from po/Makefile.in.in
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.21])

# Set the directory containing po/ (default is po/)
# This creates po/Makefile.in which becomes po/Makefile
# DATADIRNAME is set by AM_GNU_GETTEXT (usually 'share')

# ============================
# PKG-CONFIG CHECKS
# ============================

# Check for pkg-config
PKG_PROG_PKG_CONFIG([0.29])

# GTKmm 4.0 (or adjust to 3.0 if you need legacy)
PKG_CHECK_MODULES([GTKMM], [gtkmm-4.0 >= 4.0.0])
AC_SUBST([GTKMM_CFLAGS])
AC_SUBST([GTKMM_LIBS])


AC_PATH_PROG([GLIB_COMPILE_RESOURCES], [glib-compile-resources], [no])
if test "x$GLIB_COMPILE_RESOURCES" = "xno"; then
  AC_MSG_ERROR([glib-compile-resources not found. Install glib2-devel.])
fi
AC_SUBST([GLIB_COMPILE_RESOURCES])

PKG_CHECK_MODULES(XSLT, [libxslt >= 1.1.20])
AC_SUBST(XSLT_CFLAGS)

PKG_CHECK_MODULES(GLIBMM, glibmm-2.68)
AC_SUBST(GLIBMM_CFLAGS)
AC_SUBST(GLIBMM_LIBS)

PKG_CHECK_MODULES(XMLPP, libxml++-5.0)
AC_SUBST(XMLPP_CFLAGS)
AC_SUBST(XMLPP_LIBS)

PKG_CHECK_MODULES(ARCHIVE, [libarchive >= 0.28])
AC_SUBST(ARCHIVE_CFLAGS)
AC_SUBST(ARCHIVE_CFLAGS)

dnl Finally, check for libsigc++
PKG_CHECK_MODULES(LIBSIGC, sigc++-3.0)
AC_SUBST(LIBSIGC_CFLAGS)
AC_SUBST(LIBSIGC_LIBS)

AC_CHECK_LIB(pthread, pthread_create)

PKG_CHECK_MODULES(GST, [gstreamer-1.0 >= 1.18])
AC_SUBST(GST_CFLAGS)
AC_SUBST(GST_LIBS)

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

eval LORDSAWAR_DATADIR="$datadir/lw"
AC_SUBST(LORDSAWAR_DATADIR)

eval eval DATADIR="$datadir"
AC_SUBST(DATADIR)

# ============================
# OUTPUT FILES
# ============================

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    data/Makefile
    po/Makefile.in
    po/Makefile
    data/ui/Makefile
    data/ui/editor/Makefile
    data/icons/Makefile 
    data/icons/16x16/Makefile 
    data/icons/22x22/Makefile 
    data/icons/24x24/Makefile 
    data/icons/32x32/Makefile 
    data/icons/48x48/Makefile 
    data/icons/64x64/Makefile 
    data/org.nongnu.lordsawar.desktop.in
    data/armies/Makefile 
    data/armies/default/Makefile 
    data/armies/duel/Makefile 
    data/terrain/Makefile
    data/terrain/default/Makefile
    data/terrain/stark/Makefile
    data/terrain/wesnoth/Makefile
    data/shields/Makefile 
    data/shields/default/Makefile 
    data/shields/dol/Makefile 
    data/shields/plain/Makefile 
    data/buildings/Makefile 
    data/buildings/default/Makefile 
    data/buildings/dol/Makefile 
    data/various/Makefile 
    data/various/items/Makefile
    data/various/editor/Makefile
    data/various/xslt/Makefile
    data/maps/Makefile 
    data/maps/dol/Makefile 
    data/maps/duel/Makefile 
    data/music/Makefile
])

AC_OUTPUT

# Summary
AC_MSG_NOTICE([
==================================
Configuration summary for $PACKAGE_NAME $PACKAGE_VERSION
==================================

C++ Compiler:        $CXX
C++ Flags:           $CXXFLAGS
GTKmm CFlags:        $GTKMM_CFLAGS
GTKmm Libraries:     $GTKMM_LIBS

Type 'make' to build the project.
])
