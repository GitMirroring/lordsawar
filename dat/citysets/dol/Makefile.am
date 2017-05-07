MAINTAINERCLEANFILES = Makefile.in

lordsawarcitysetdir = $(datadir)/lordsawar/citysets

dol.lwc: dol.lwc.in
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/dol.lwc.in $(builddir)/dol.lwc && \
	$(top_srcdir)/create-lw-file $(builddir)/dol.lwc $(IMAGES)

DISTCLEANFILES = dol.lwc 

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
