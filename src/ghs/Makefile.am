#   Copyright (C) 2011, 2015 Ben Asselstine
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

if WANT_GHS
bin_PROGRAMS = lordsawar-game-host-server lordsawar-game-host-client

lordsawar_game_host_client_SOURCES = ghs-client-main.cpp \
				     ghs-client-tool.cpp ghs-client-tool.h

lordsawar_game_host_client_LDADD = \
                                  -L$(top_builddir)/src \
                                  $(top_builddir)/src/liblordsawar.la \
                                  $(top_builddir)/src/liblordsawargamelist.la \
                                  $(top_builddir)/src/liblordsawargamehost.la \
                                  $(top_builddir)/src/liblordsawarnet.la \
                                  $(GSTREAMER_LIBS) \
                                  $(LIBSIGC_LIBS) \
                                  $(GTKMM_LIBS) \
                                  $(XSLT_LIBS) \
                                  $(XMLPP_LIBS) \
                                  $(ARCHIVE_LIBS) \
                                  -lz

lordsawar_game_host_server_SOURCES = main.cpp \
  gamehost-server.cpp gamehost-server.h 

lordsawar_game_host_server_LDADD = \
                                  -L$(top_builddir)/src \
                                  $(top_builddir)/src/liblordsawar.la \
                                  $(top_builddir)/src/liblordsawargamelist.la \
                                  $(top_builddir)/src/liblordsawargamehost.la \
                                  $(top_builddir)/src/liblordsawarnet.la \
                                  $(GSTREAMER_LIBS) \
                                  $(LIBSIGC_LIBS) \
                                  $(GTKMM_LIBS) \
                                  $(XSLT_LIBS) \
                                  $(XMLPP_LIBS) \
                                  $(ARCHIVE_LIBS) \
                                  -lz
endif


localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@


AM_CXXFLAGS = -Wall -Wshadow -Wextra -pedantic -Wno-deprecated-declarations \
	      -Wno-deprecated-copy -DPO_PATH=\"$(prefix)/share/locale/\" \
	      -std=c++11
AM_CPPFLAGS = \
    $(SDL_CFLAGS) \
    $(LIBSIGC_CFLAGS) \
    $(GTKMM_CFLAGS) \
    $(XMLPP_CFLAGS) \
    $(XSLT_CFLAGS) \
    -DLORDSAWAR_DATADIR='"$(LORDSAWAR_DATADIR)"' \
    -I$(top_srcdir) -I$(top_srcdir)/src

EXTRA_DIST = README
