# Copyright (C) 2026 Ben Asselstine
# This file is licensed under the terms of the GNU GPL version 3 or later.
MAINTAINERCLEANFILES = Makefile.in

lordsawarmapdir = $(pkgdatadir)/maps

dol.map: dol.map.in
	rm $(builddir)/dol.map -f 2>/dev/null && \
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/dol.map.in $(builddir)/dol.map && \
	$(top_srcdir)/create-lw-file $(builddir)/dol.map $(builddir)/../../armies/default/default.lwa $(builddir)/../../buildings/default/default.lwc $(builddir)/../../shields/default/default.lws $(builddir)/../../terrain/default/default.lwt

CLEANFILES = dol.map 

lordsawarmap_DATA = dol.map

EXTRA_DIST = \
	dol.map.in \
	README
