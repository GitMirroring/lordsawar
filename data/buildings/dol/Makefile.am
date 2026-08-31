# Copyright (C) 2026 Ben Asselstine
# This file is licensed under the terms of the GNU GPL version 3 or later.
MAINTAINERCLEANFILES = Makefile.in

lordsawarcitysetdir = $(pkgdatadir)/buildings

dol.lwc: dol.lwc.in
	rm $(builddir)/dol.lwc -f 2>/dev/null && \
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/dol.lwc.in $(builddir)/dol.lwc && \
	$(top_srcdir)/create-lw-file $(builddir)/dol.lwc $(IMAGES)

CLEANFILES = dol.lwc 

lordsawarcityset_DATA = dol.lwc

IMAGES = \
	$(srcdir)/castle_razed.png \
	$(srcdir)/castles.png \
	$(srcdir)/port.png \
	$(srcdir)/ruin.png \
	$(srcdir)/signpost.png \
	$(srcdir)/temples.png \
	$(srcdir)/towers.png

EXTRA_DIST = \
	dol.lwc.in \
	README \
	$(IMAGES)
