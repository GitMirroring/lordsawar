MAINTAINERCLEANFILES = Makefile.in

lordsawarmapdir = $(datadir)/lordsawar/map

dol.map: dol.map.in
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/dol.map.in $(builddir)/dol.map
	$(top_srcdir)/create-lw-file $(builddir)/dol.map $(top_builddir)/dat/army/default/default.lwa $(top_builddir)/dat/citysets/default/default.lwc $(top_builddir)/dat/shield/default/default.lws $(top_builddir)/dat/tilesets/default/default.lwt

DISTCLEANFILES = dol.map 

lordsawarmap_DATA = dol.map

EXTRA_DIST = \
	dol.map.in \
	README
