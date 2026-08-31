MAINTAINERCLEANFILES = Makefile.in

lordsawarmapdir = $(datadir)/lordsawar/map

duel.map: duel.map.in
	cp $(srcdir)/duel.map.in $(builddir)/duel.map
	$(top_srcdir)/create-lw-file $(builddir)/duel.map $(top_builddir)/dat/army/duel/duel.lwa $(top_builddir)/dat/citysets/default/default.lwc $(top_builddir)/dat/shield/default/default.lws $(top_builddir)/dat/tilesets/default/default.lwt

DISTCLEANFILES = duel.map 

lordsawarmap_DATA = duel.map

EXTRA_DIST = \
	duel.map.in \
	README
