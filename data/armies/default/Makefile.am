# Copyright (C) 2026 Ben Asselstine
# This file is licensed under the terms of the GNU GPL version 3 or later.
MAINTAINERCLEANFILES = Makefile.in

lordsawararmydir = $(pkgdatadir)/armies

default.lwa: default.lwa.in $(IMAGES)
	rm -f $(builddir)/default.lwa 2>/dev/null && \
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/default.lwa.in $(builddir)/default.lwa && \
	$(top_srcdir)/create-lw-file $(builddir)/default.lwa $(IMAGES)

CLEANFILES = default.lwa 

lordsawararmy_DATA = default.lwa

IMAGES = \
	$(srcdir)/archers.png \
	$(srcdir)/archons.png \
	$(srcdir)/catapults.png \
	$(srcdir)/daemons.png \
	$(srcdir)/devils.png \
	$(srcdir)/dragons.png \
	$(srcdir)/dwarves.png \
	$(srcdir)/elementals.png \
	$(srcdir)/elephants.png \
	$(srcdir)/ghosts.png \
	$(srcdir)/giantbats.png \
	$(srcdir)/giants.png \
	$(srcdir)/giantworms.png \
	$(srcdir)/griffins.png \
	$(srcdir)/heavycav.png \
	$(srcdir)/heavyinf.png \
	$(srcdir)/hero.png \
	$(srcdir)/items.png \
	$(srcdir)/lightcav.png \
	$(srcdir)/lightinf.png \
	$(srcdir)/minotaurs.png \
	$(srcdir)/orcishmob.png \
	$(srcdir)/pegasi.png \
	$(srcdir)/pikeman.png \
	$(srcdir)/plantedstandard.png \
	$(srcdir)/scouts.png \
	$(srcdir)/spiders.png \
	$(srcdir)/stackship.png \
	$(srcdir)/unicorns.png \
	$(srcdir)/wizards.png \
	$(srcdir)/wolfriders.png \
	$(srcdir)/selector.png \
	$(srcdir)/small_selector.png

EXTRA_DIST = \
	default.lwa.in \
	README \
	$(IMAGES)
