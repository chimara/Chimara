pkglib_LTLIBRARIES = bocfel.la
bocfel_la_SOURCES = blorb.c blorb.h branch.c branch.h dict.c dict.h glkstart.c \
	help.h iff.c iff.h io.c io.h math.c math.h memory.c memory.h meta.c meta.h \
	objects.c objects.h osdep.c osdep.h process.c process.h random.c random.h \
	screen.c screen.h sound.c sound.h stack.c stack.h unicode.c unicode.h \
	util.c util.h zoom.c zoom.h zterp.c zterp.h
bocfel_la_CPPFLAGS = -DZTERP_GLK -DZTERP_UNIX \
	-I$(top_srcdir) -I$(top_srcdir)/libchimara
bocfel_la_CFLAGS = -std=c99 $(AM_CFLAGS)
bocfel_la_LDFLAGS = -module $(PLUGIN_LIBTOOL_FLAGS)

bocfeldocdir = $(datadir)/doc/$(PACKAGE)/bocfel
dist_bocfeldoc_DATA = BUILDING COPYING.GPLv2 COPYING.GPLv3 README

-include $(top_srcdir)/git.mk
