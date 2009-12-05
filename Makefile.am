## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = libchimara interpreters player tests docs po

chimaradocdir = $(datadir)/doc/chimara
dist_chimaradoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	NEWS

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = chimara.pc chimara-plugin.pc

EXTRA_DIST = chimara.pc.in chimara-plugin.pc.in chimara.spec.in

DISTCLEANFILES = chimara.pc chimara-plugin.pc chimara.spec

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
