#   Copyright (C) 2007, 2008 Ole Laursen
#   Copyright (C) 2007, 2008, 2009, 2010, 2015, 2017, 2020 Ben Asselstine
#   Copyright (C) 2008 Barry deFreese
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

noinst_LTLIBRARIES = libshieldsetwindow.la libarmysetwindow.la libcitysetwindow.la libtilesetwindow.la libeditutil.la libeditor.la

libeditutil_la_SOURCES = \
	editor-quit-dialog.cpp editor-quit-dialog.h \
	editor-save-changes-dialog.cpp editor-save-changes-dialog.h \
	lw-editor-dialog.cpp lw-editor-dialog.h \
	past-chooser.cpp past-chooser.h \
	image-file-filter.cpp image-file-filter.h

libshieldsetwindow_la_SOURCES = \
	shieldset-window.cpp shieldset-window.h \
	shieldset-info-dialog.cpp shieldset-info-dialog.h \
	masked-image-editor-dialog.cpp masked-image-editor-dialog.h

libarmysetwindow_la_SOURCES = \
	armyset-window.cpp armyset-window.h  \
	armyset-info-dialog.cpp armyset-info-dialog.h \
	masked-image-editor-dialog.cpp masked-image-editor-dialog.h \
	per-player-masked-image-editor-dialog.cpp per-player-masked-image-editor-dialog.h \
	image-editor-dialog.cpp image-editor-dialog.h 

libcitysetwindow_la_SOURCES = \
	cityset-window.cpp cityset-window.h  \
	cityset-info-dialog.cpp cityset-info-dialog.h \
	image-editor-dialog.cpp image-editor-dialog.h 

libtilesetwindow_la_SOURCES = \
	tileset-window.cpp tileset-window.h \
	tileset-info-dialog.cpp tileset-info-dialog.h \
	tile-preview-dialog.cpp tile-preview-dialog.h \
	tile-preview-scene.cpp tile-preview-scene.h \
	tileset-selector-editor-dialog.cpp tileset-selector-editor-dialog.h \
	tileset-flag-editor-dialog.cpp tileset-flag-editor-dialog.h \
	tileset-explosion-picture-editor-dialog.cpp \
	tileset-explosion-picture-editor-dialog.h \
	tilestyle-organizer-dialog.cpp tilestyle-organizer-dialog.h \
	tileset-smallmap-building-colors-dialog.cpp \
	tileset-smallmap-building-colors-dialog.h\
	image-editor-dialog.cpp image-editor-dialog.h

libeditor_la_SOURCES =\
	main-window.cpp main-window.h \
	signpost-editor-dialog.cpp signpost-editor-dialog.h \
	temple-editor-dialog.cpp temple-editor-dialog.h \
	ruin-editor-dialog.cpp ruin-editor-dialog.h \
	stack-editor-dialog.cpp stack-editor-dialog.h \
	select-army-dialog.cpp select-army-dialog.h \
	players-dialog.cpp players-dialog.h \
	city-editor-dialog.cpp city-editor-dialog.h \
	map-info-dialog.cpp map-info-dialog.h \
	new-map-dialog.cpp new-map-dialog.h \
	switch-sets-dialog.cpp switch-sets-dialog.h \
	editorbigmap.cpp editorbigmap.h \
	editablesmallmap.cpp editablesmallmap.h \
	smallmap-editor-dialog.cpp smallmap-editor-dialog.h \
	itemlist-dialog.cpp itemlist-dialog.h \
	reward-editor-dialog.cpp reward-editor-dialog.h \
	select-item-dialog.cpp select-item-dialog.h \
	rewardlist-dialog.cpp rewardlist-dialog.h \
	select-hidden-ruin-dialog.cpp select-hidden-ruin-dialog.h \
	RenamableLocation.cpp RenamableLocation.h  \
	backpack-editor-dialog.cpp backpack-editor-dialog.h \
	item-editor-dialog.cpp item-editor-dialog.h \
	planted-standard-editor-dialog.cpp planted-standard-editor-dialog.h \
	hero-editor-dialog.cpp hero-editor-dialog.h \
	editor-splash-window.cpp editor-splash-window.h \
	fight-order-editor-dialog.cpp fight-order-editor-dialog.h \
	road-editor-tip.cpp road-editor-tip.h \
	stone-editor-dialog.cpp stone-editor-dialog.h \
	battle-calculator-dialog.cpp battle-calculator-dialog.h \
	media-dialog.cpp media-dialog.h \
	validation-dialog.cpp validation-dialog.h \
	heroes-dialog.cpp heroes-dialog.h \
	keeper-editor-dialog.cpp keeper-editor-dialog.h \
	army-chooser-button.cpp army-chooser-button.h

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@


AM_CXXFLAGS = -Wall -Wshadow -Wextra -pedantic -Wno-deprecated-declarations \
	      -Wno-deprecated-copy -std=c++11
AM_CPPFLAGS = $(GTKMM_CFLAGS) \
	      $(XMLPP_CFLAGS) \
    $(XSLT_CFLAGS) \
    -DLORDSAWAR_DATADIR='"$(LORDSAWAR_DATADIR)"' -I$(top_srcdir) -I$(top_srcdir)/src -I$(top_srcdir)/src/gui/
