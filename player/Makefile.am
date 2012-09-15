AM_CFLAGS = -Wall
AM_CPPFLAGS = -I$(top_srcdir)

PLUGIN_LIBTOOL_FLAGS=-module -avoid-version -export-symbols-regex "^glk_main$$"

dist_pkgdata_DATA = chimara.ui chimara.menus style.css
bin_PROGRAMS = chimara

chimara_SOURCES = main.c callbacks.c preferences.c preferences.h error.c error.h
chimara_CPPFLAGS = $(AM_CPPFLAGS) \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\"
chimara_CFLAGS = @PLAYER_CFLAGS@ $(AM_CFLAGS)
chimara_LDADD = @PLAYER_LIBS@ $(top_builddir)/libchimara/libchimara.la

gsettings_SCHEMAS = org.chimara-if.gschema.xml
@GSETTINGS_RULES@

CLEANFILES = config.pyc
DISTCLEANFILES = config.py

EXTRA_DIST = \
	$(gsettings_SCHEMAS) \
	config.py \
	player.py

-include $(top_srcdir)/git.mk
