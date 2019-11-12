AM_CFLAGS = -Wall
AM_CPPFLAGS = -I$(top_srcdir)

PLUGIN_LIBTOOL_FLAGS=-module -avoid-version -export-symbols-regex "^glk_main$$"

bin_PROGRAMS = chimara

chimara_SOURCES = \
	actions.c actions.h \
	callbacks.c callbacks.h \
	error.c error.h \
	main.c \
	preferences.c preferences.h \
	resources.c \
	$(NULL)
chimara_CPPFLAGS = $(AM_CPPFLAGS) \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	$(NULL)
chimara_CFLAGS = @PLAYER_CFLAGS@ $(AM_CFLAGS)
chimara_LDADD = @PLAYER_LIBS@ $(top_builddir)/libchimara/libchimara.la

resource_file = $(srcdir)/resources/org.chimara-if.player.gresource.xml
resource_deps := $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies \
	--sourcedir $(srcdir)/resources $(resource_file))

resources.c: $(resource_file) $(resource_deps)
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCES) --target="$@" \
		--sourcedir $(srcdir)/resources --generate-source "$<"

gsettings_SCHEMAS = org.chimara-if.gschema.xml
@GSETTINGS_RULES@

BUILT_SOURCES = resources.c

EXTRA_DIST = \
	$(gsettings_SCHEMAS) \
	$(resource_file) \
	$(resource_deps) \
	$(NULL)

-include $(top_srcdir)/git.mk
