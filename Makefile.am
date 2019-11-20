## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = libchimara interpreters player tests docs po

chimaradocdir = $(datadir)/doc/chimara
dist_chimaradoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	NEWS

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = chimara.pc chimara-plugin.pc

EXTRA_DIST = \
	chimara.pc.in
	chimara-plugin.pc.in
	chimara.spec.in
	m4/introspection.m4

DISTCLEANFILES = chimara.pc chimara-plugin.pc chimara.spec

macros_ignore = \
	codeset extern-inline fcntl-o gettext glibc2 glibc21 gtk-doc iconv intdiv0 \
	intl intldir intlmacosx intmax inttypes_h inttypes-pri lcmessage lib-ld \
	lib-link lib-prefix lock longlong nls po printf-posix progtest size_max \
	stdint_h threadlib uintmax_t visibility wchar_t wint_t xsize \
	$(NULL)

MAINTAINERCLEANFILES = \
	$(GITIGNORE_MAINTAINERCLEANFILES_TOPLEVEL) \
	$(GITIGNORE_MAINTAINERCLEANFILES_MAKEFILE_IN) \
	$(GITIGNORE_MAINTAINERCLEANFILES_M4_LIBTOOL) \
	ABOUT-NLS INSTALL config.rpath gtk-doc.make tap-driver.sh \
	$(addprefix 'm4/',$(addsuffix '.m4', $(macros_ignore))) \
	$(NULL)

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
