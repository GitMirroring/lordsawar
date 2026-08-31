# Copyright (C) 2026 Ben Asselstine
# This file is licensed under the terms of the GNU GPL version 3 or later.
MAINTAINERCLEANFILES = Makefile.in

lordsawarcitysetdir = $(pkgdatadir)/buildings

default.lwc: default.lwc.in $(IMAGES)
	rm $(builddir)/default.lwc -f 2>/dev/null && \
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/default.lwc.in $(builddir)/default.lwc && \
	$(top_srcdir)/create-lw-file $(builddir)/default.lwc $(IMAGES)

CLEANFILES = default.lwc 

lordsawarcityset_DATA = default.lwc

IMAGES = \
	$(srcdir)/castle_razed.png \
	$(srcdir)/castles.png \
	$(srcdir)/port.png \
	$(srcdir)/ruin.png \
	$(srcdir)/signpost.png \
	$(srcdir)/temples.png \
	$(srcdir)/towers.png

EXTRA_DIST = \
	default.lwc.in \
	README \
	$(IMAGES)
