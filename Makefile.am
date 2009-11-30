## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = libchimara interpreters tests docs po player

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
	chimara.pc.in chimara-plugin.pc.in chimara.spec.in

DISTCLEANFILES = chimara.pc chimara-plugin.pc chimara.spec \
				 intltool-extract intltool-merge intltool-update

ACLOCAL_AMFLAGS = -I m4

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

if BUILDING_RPM
EXTRA_DIST += chimara.spec
rpm: chimara.spec dist
	$(ECHO) "Building RPM package..."; \
	$(RPMBUILD) -ta --clean $(PACKAGE)-$(VERSION).tar.gz
else
rpm:
	@$(ECHO) \
	"To build an RPM package, you must run configure with --enable-rpm."; \
	exit 1
endif
