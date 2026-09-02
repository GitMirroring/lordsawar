# Copyright (C) 2026 Ben Asselstine
# This file is licensed under the terms of the GNU GPL version 3 or later.

SUBDIRS = armies terrain various music shields buildings maps icons ui
MANTAINERCLEANFILES= Makefile.in

lordsawardatadir = $(pgadatadir)

@INTLTOOL_XML_RULE@

lordsawardata_DATA = \
    citynames.xml \
    heronames.xml \
    templenames.xml \
    ruinnames.xml \
    signposts.xml

DISTCLEANFILES = citynames.xml heronames.xml templenames.xml ruinnames.xml signposts.xml org.nongnu.lordsawar.desktop org.nongnu.lordsawar.desktop.in

CLEANFILES=desktop-foo desktop-bar

org.nongnu.lordsawar.desktop: org.nongnu.lordsawar.desktop.in
		sed -e 's|.[{]prefix[}]|${prefix}|' org.nongnu.lordsawar.desktop.in > desktop-foo \
		&& $(INTLTOOL_MERGE) $(top_srcdir)/po desktop-foo $(@) -d -u -c $(top_builddir)/po/.intltool-merge-cache \
		&& rm -r desktop-foo

lwdesktopdir= $(datadir)/applications
lwdesktop_DATA= org.nongnu.lordsawar.desktop

appdatadir=$(datadir)/appdata
appdata_DATA=org.nongnu.lordsawar-appdata.xml

EXTRA_DIST = \
    README \
    org.nongnu.lordsawar-appdata.xml \
    org.nongnu.lordsawar.desktop.in.in \
    resources.gresource.xml \
    citynames.xml.in \
    heronames.xml.in \
    templenames.xml.in \
    ruinnames.xml.in \
    signposts.xml.in
