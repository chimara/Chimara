## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src po docs

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

DISTCLEANFILES = intltool-extract intltool-merge intltool-update

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc
