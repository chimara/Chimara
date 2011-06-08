## Process this file with automake to produce Makefile.in
## Created by Anjuta

if TARGET_ILIAD
SUBDIRS = libchimara interpreters player po
else
SUBDIRS = libchimara interpreters player tests docs po
endif

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

MAINTAINERCLEANFILES = ABOUT-NLS INSTALL aclocal.m4 compile config.guess \
	config.h.in config.rpath config.sub depcomp gtk-doc.make install-sh \
	intltool-extract.in 	intltool-merge.in intltool-update.in ltmain.sh missing \
	ylwrap m4 autom4te.cache

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

-include $(top_srcdir)/git.mk
