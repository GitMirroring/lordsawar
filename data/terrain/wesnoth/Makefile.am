# Copyright (C) 2026 Ben Asselstine
# This file is licensed under the terms of the GNU GPL version 3 or later.
MAINTAINERCLEANFILES = Makefile.in

lordsawartilesetdir = $(pkgdatadir)/terrain

wesnoth.lwt: wesnoth.lwt.in
	rm $(builddir)/wesnoth.lwt -f 2>/dev/null && \
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/wesnoth.lwt.in $(builddir)/wesnoth.lwt && \
	$(top_srcdir)/create-lw-file $(builddir)/wesnoth.lwt $(IMAGES)

CLEANFILES = wesnoth.lwt 

lordsawartileset_DATA = wesnoth.lwt

IMAGES = \
	$(srcdir)/bridges.png \
	$(srcdir)/explosion.png \
	$(srcdir)/flags.png \
	$(srcdir)/fog.png \
	$(srcdir)/forest.png \
	$(srcdir)/grass.png \
	$(srcdir)/hills.png \
	$(srcdir)/mountains.png \
	$(srcdir)/roads.png \
	$(srcdir)/stones.png \
	$(srcdir)/selector.png \
	$(srcdir)/small_selector.png \
	$(srcdir)/swamp.png \
	$(srcdir)/water.png \
	$(srcdir)/flight_movebonus.png \
	$(srcdir)/water_movebonus.png \
	$(srcdir)/forest_movebonus.png \
	$(srcdir)/hills_movebonus.png \
	$(srcdir)/mountains_movebonus.png \
	$(srcdir)/swamp_movebonus.png

EXTRA_DIST = \
	wesnoth.lwt.in \
	README \
	$(IMAGES)
