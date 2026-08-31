MAINTAINERCLEANFILES = Makefile.in

lordsawararmydir = $(datadir)/lordsawar/army

duel.lwa: duel.lwa.in
	cp $(srcdir)/duel.lwa.in $(builddir)/duel.lwa && \
	$(top_srcdir)/create-lw-file $(builddir)/duel.lwa $(IMAGES)

DISTCLEANFILES = duel.lwa 

lordsawararmy_DATA = duel.lwa

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
	duel.lwa.in \
	README \
	$(IMAGES)
