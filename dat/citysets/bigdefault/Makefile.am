MAINTAINERCLEANFILES = Makefile.in

lordsawarcitysetdir = $(datadir)/lordsawar/citysets

bigdefault.lwc: bigdefault.lwc.in
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/bigdefault.lwc.in $(builddir)/bigdefault.lwc && \
	$(top_srcdir)/create-lw-file $(builddir)/bigdefault.lwc $(IMAGES)

DISTCLEANFILES = bigdefault.lwc 

lordsawarcityset_DATA = bigdefault.lwc

IMAGES = \
	$(srcdir)/castle_razed.png \
	$(srcdir)/castles.png \
	$(srcdir)/port.png \
	$(srcdir)/ruin.png \
	$(srcdir)/signpost.png \
	$(srcdir)/temples.png \
	$(srcdir)/towers.png

EXTRA_DIST = \
	bigdefault.lwc.in \
	README \
	$(IMAGES)
