#   Copyright (C) 2016 Ben Asselstine
# 
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
# 
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Library General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
#   02110-1301, USA.
MAINTAINERCLEANFILES= Makefile.in

bin_PROGRAMS = lordsawar-import lordsawar-upgrade-file

lordsawar_import_SOURCES = import.cpp
lordsawar_import_LDADD = $(top_builddir)/src/gui/liblwgui.la \
                        $(top_builddir)/src/liblordsawargfx.la \
                        $(top_builddir)/src/liblordsawar.la \
    			$(GSTREAMER_LIBS) \
			$(GTKMM_LIBS) \
			$(XMLPP_LIBS) \
			$(XSLT_LIBS) \
			$(ARCHIVE_LIBS) \
			-lz \
			-L$(top_builddir)/src

lordsawar_upgrade_file_SOURCES = upgrade-file.cpp

lordsawar_upgrade_file_LDADD = $(top_builddir)/src/liblordsawar.la \
    $(top_builddir)/src/liblordsawargamelist.la \
    $(top_builddir)/src/liblordsawargamehost.la \
    $(GSTREAMER_LIBS) \
    $(GTKMM_LIBS) \
    $(XMLPP_LIBS) \
    $(XSLT_LIBS) \
    $(ARCHIVE_LIBS) \
    $(LIBSIGC_LIBS) \
    -lz

lordsawar_upgrade_file_DEPENDENCIES = \
  $(top_builddir)/src/liblordsawar.la \
  $(top_builddir)/src/liblordsawargamelist.la \
  $(top_builddir)/src/liblordsawargamehost.la

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

AM_CXXFLAGS = -Wall -Wshadow -Wextra -pedantic -Wno-deprecated-declarations \
	      -Wno-deprecated-copy -std=c++11
AM_CPPFLAGS = $(GTKMM_CFLAGS) \
	      $(XMLPP_CFLAGS) \
    $(XSLT_CFLAGS) \
    -DLORDSAWAR_DATADIR='"$(LORDSAWAR_DATADIR)"' -I$(top_srcdir) -I$(top_srcdir)/src -I$(top_srcdir)/src/gui/
