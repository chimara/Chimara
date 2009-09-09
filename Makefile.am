## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = libchimara interpreters tests docs po

chimaradocdir = ${prefix}/doc/chimara
dist_chimaradoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = chimara.pc chimara-plugin.pc

EXTRA_DIST = intltool-extract.in intltool-merge.in intltool-update.in \
	chimara.pc.in chimara-plugin.pc.in

DISTCLEANFILES = chimara.pc chimara-plugin.pc \
				 intltool-extract intltool-merge intltool-update

ACLOCAL_AMFLAGS = -I m4

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc
