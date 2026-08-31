# Copyright (C) 2026 Ben Asselstine
# This file is licensed under the terms of the GNU GPL version 3 or later.
MAINTAINERCLEANFILES = Makefile.in

lordsawartilesetdir = $(pkgdatadir)/terrain

default.lwt: default.lwt.in
	rm $(builddir)/default.lwt -f 2>/dev/null && \
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/default.lwt.in $(builddir)/default.lwt && \
	$(top_srcdir)/create-lw-file $(builddir)/default.lwt $(IMAGES)

CLEANFILES = default.lwt 

lordsawartileset_DATA = default.lwt

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
	default.lwt.in \
	README \
	$(IMAGES)
