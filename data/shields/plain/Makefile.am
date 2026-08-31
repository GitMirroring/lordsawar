# Copyright (C) 2026 Ben Asselstine
# This file is licensed under the terms of the GNU GPL version 3 or later.
MAINTAINERCLEANFILES = Makefile.in

lordsawarshielddir = $(pkgdatadir)/shields

IMAGES = \
	$(srcdir)/large-neutral-shield.png \
	$(srcdir)/large-shield.png \
	$(srcdir)/medium-neutral-shield.png \
	$(srcdir)/medium-shield.png \
	$(srcdir)/small-neutral-shield.png \
	$(srcdir)/small-shield.png \
	$(srcdir)/left-tartan.png \
	$(srcdir)/center-tartan.png \
	$(srcdir)/right-tartan.png

plain.lws: plain.lws.in $(IMAGES)
	rm $(builddir)/plain.lws -f 2>/dev/null && \
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/plain.lws.in $(builddir)/plain.lws && \
	$(top_srcdir)/create-lw-file $(builddir)/plain.lws $(IMAGES)

CLEANFILES = plain.lws 

lordsawarshield_DATA = plain.lws

EXTRA_DIST = \
	plain.lws.in \
	README \
	$(IMAGES)
