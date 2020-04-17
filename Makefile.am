SUBDIRS = src dat po doc help
MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~

lordsawarconfigdir = $(LORDSAWAR_DATADIR)

EXTRA_DIST = DEPENDENCIES README ${top_builddir}/po/lordsawar.pot \
	     autogen.sh create-lw-file copy-gnome-icons.sh WINDOWS \
	     make-windows-installer.sh

DISTCLEANFILES = po/.intltool-merge-cache

ACLOCAL_AMFLAGS = -I m4
