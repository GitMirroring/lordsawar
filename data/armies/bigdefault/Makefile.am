MAINTAINERCLEANFILES = Makefile.in

lordsawararmydir = $(datadir)/lordsawar/army

bigdefault.lwa: bigdefault.lwa.in
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/bigdefault.lwa.in $(builddir)/bigdefault.lwa && \
	$(top_srcdir)/create-lw-file $(builddir)/bigdefault.lwa $(IMAGES)

DISTCLEANFILES = bigdefault.lwa 

lordsawararmy_DATA = bigdefault.lwa

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
	$(srcdir)/wolfriders.png

EXTRA_DIST = \
	bigdefault.lwa.in \
	README \
	$(IMAGES)
