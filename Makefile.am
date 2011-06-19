## Process this file with automake to produce Makefile.in
## Created by Anjuta

if TARGET_ILIAD
SUBDIRS = libchimara interpreters player po
else
SUBDIRS = libchimara interpreters player tests docs po babel
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

EXTRA_DIST = \
	chimara.pc.in
	chimara-plugin.pc.in
	chimara.spec.in
	m4/introspection.m4

DISTCLEANFILES = chimara.pc chimara-plugin.pc chimara.spec

macros_ignore = codeset gettext glibc2 glibc21 gtk-doc iconv intdiv0 intl \
	intldir intlmacosx intltool intmax inttypes_h inttypes-pri lcmessage \
	lib-ld lib-link lib-prefix libtool lock longlong lt~obsolete ltoptions \
	ltsugar ltversion nls po printf-posix progtest size_max stdint_h uintmax_t \
	visibility wchar_t wint_t xsize

MAINTAINERCLEANFILES = ABOUT-NLS INSTALL aclocal.m4 compile config.guess \
	config.h.in config.rpath config.sub depcomp gtk-doc.make install-sh \
	intltool-extract.in intltool-merge.in intltool-update.in ltmain.sh missing \
	ylwrap autom4te.cache \
	$(addprefix 'm4/',$(addsuffix '.m4', $(macros_ignore)))

ACLOCAL_AMFLAGS = -I m4

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc --enable-introspection

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
