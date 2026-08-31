# Copyright (C) 2026 Ben Asselstine
# This file is licensed under the terms of the GNU GPL version 3 or later.
MAINTAINERCLEANFILES = Makefile.in

lordsawarmapdir = $(pkgdatadir)/maps

duel.map: duel.map.in
	rm $(builddir)/duel.map -f 2>/dev/null && \
	cp $(srcdir)/duel.map.in $(builddir)/duel.map && \
	$(top_srcdir)/create-lw-file $(builddir)/duel.map $(builddir)/../../armies/duel/duel.lwa $(builddir)/../../buildings/default/default.lwc $(builddir)/../../shields/default/default.lws $(builddir)/../../terrain/default/default.lwt

CLEANFILES = duel.map 

lordsawarmap_DATA = duel.map

EXTRA_DIST = \
	duel.map.in \
	README
