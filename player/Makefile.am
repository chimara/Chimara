AM_CFLAGS = -Wall
AM_CPPFLAGS = -I$(top_srcdir)

PLUGIN_LIBTOOL_FLAGS=-module -avoid-version -export-symbols-regex "^glk_main$$"

if TARGET_ILIAD

chimara_iliad_SOURCES = iliad.c
chimara_iliad_CFLAGS = @TEST_CFLAGS@ $(AM_CFLAGS)
chimara_iliad_LDADD = @TEST_LIBS@ $(top_builddir)/libchimara/libchimara.la

bin_PROGRAMS = chimara_iliad

else

dist_pkgdata_DATA = chimara.ui chimara.menus 
bin_PROGRAMS = chimara

chimara_SOURCES = main.c callbacks.c error.c error.h
chimara_CPPFLAGS = $(AM_CPPFLAGS) \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\"
chimara_CFLAGS = @TEST_CFLAGS@ $(AM_CFLAGS)
chimara_LDADD = @TEST_LIBS@ $(top_builddir)/libchimara/libchimara.la

endif
