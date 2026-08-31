MAINTAINERCLEANFILES = Makefile.in

lordsawarcitysetdir = $(datadir)/lordsawar/citysets

bigdol.lwc: bigdol.lwc.in
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/bigdol.lwc.in $(builddir)/bigdol.lwc && \
	$(top_srcdir)/create-lw-file $(builddir)/bigdol.lwc $(IMAGES)

DISTCLEANFILES = bigdol.lwc 

lordsawarcityset_DATA = bigdol.lwc

IMAGES = \
	$(srcdir)/castle_razed.png \
	$(srcdir)/castles.png \
	$(srcdir)/port.png \
	$(srcdir)/ruin.png \
	$(srcdir)/signpost.png \
	$(srcdir)/temples.png \
	$(srcdir)/towers.png

EXTRA_DIST = \
	bigdol.lwc.in \
	README \
	$(IMAGES)
