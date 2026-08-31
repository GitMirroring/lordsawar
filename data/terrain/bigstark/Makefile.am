MAINTAINERCLEANFILES = Makefile.in

lordsawartilesetdir = $(datadir)/lordsawar/tilesets

bigstark.lwt: bigstark.lwt.in
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/bigstark.lwt.in $(builddir)/bigstark.lwt && \
	$(top_srcdir)/create-lw-file $(builddir)/bigstark.lwt $(IMAGES)

DISTCLEANFILES = bigstark.lwt 

lordsawartileset_DATA = bigstark.lwt

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
	bigstark.lwt.in \
	README \
	$(IMAGES)
