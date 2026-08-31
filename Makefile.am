# Copyright (C) 2015, 2016, 2020, 2026 Ben Asselstine
# This file is licensed under the terms of the GNU GPL version 3 or later.

SUBDIRS=src po data
ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = build-aux/config.rpath create-lw-file # config.rpath needed by gettext

update-pot:
	cd po && \
	cat POTFILES.in | sort > POTFILES.in.sorted && \
        cp POTFILES.in.sorted POTFILES.in && \
        rm POTFILES.in.sorted && \
	touch $(PACKAGE).pot && \
	rm $(PACKAGE).pot \
	&& $(MAKE) $(PACKAGE).pot

CLEANFILES = ./po/.intltool-merge-cache.lock
