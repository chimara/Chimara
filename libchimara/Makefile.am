## Process this file with automake to produce Makefile.in

AM_CFLAGS = -Wall

lib_LTLIBRARIES = libchimara.la

libchimara_la_SOURCES = \
	abort.c abort.h \
	case.c \
	charset.c charset.h \
	chimara-glk.c chimara-glk.h chimara-glk-private.h \
	chimara-if.c chimara-if.h \
	chimara-marshallers.c chimara-marshallers.h \
	datetime.c \
	dispatch.c \
	event.c event.h \
	fileref.c fileref.h \
	garglk.c garglk.h \
	gestalt.c \
	graphics.c graphics.h \
	gi_blorb.c gi_blorb.h \
	gi_dispa.c gi_dispa.h \
	glk.c glk.h \
	glkstart.h \
	glkunix.c glkunix.h \
	hyperlink.c hyperlink.h \
	init.c init.h \
	input.c input.h \
	magic.c magic.h \
	mouse.c mouse.h \
	pager.c pager.h \
	resource.c resource.h \
	schannel.c schannel.h \
	stream.c stream.h \
	strio.c strio.h \
	style.c style.h \
	timer.c timer.h \
	window.c window.h
libchimara_la_CPPFLAGS = $(AM_CPPFLAGS) \
	-DG_LOG_DOMAIN=\"Chimara\" \
	-DLOCALEDIR=\""$(datadir)/locale"\" \
	-DPLUGINDIR=\""$(pkglibdir)"\" \
	-DPLUGINSOURCEDIR=\""$(abs_builddir)/../interpreters"\" \
	-I$(top_srcdir)
libchimara_la_CFLAGS = @CHIMARA_CFLAGS@ $(AM_CFLAGS)
libchimara_la_LIBADD = @CHIMARA_LIBS@
libchimara_la_LDFLAGS = -version-info $(LT_VERSION_INFO) \
	-no-undefined \
	-export-symbols-regex "^(glk|chimara|glkunix|giblorb|gidispatch|garglk)_"
libchimara_includedir = $(includedir)/chimara/libchimara
libchimara_include_HEADERS = \
	chimara-glk.h \
	chimara-if.h \
	glk.h \
	glkstart.h \
	gi_blorb.h \
	gi_dispa.h \
	garglk.h

BUILT_SOURCES = chimara-marshallers.c chimara-marshallers.h
chimara-marshallers.c: chimara-marshallers.txt
	$(AM_V_GEN)glib-genmarshal --body --prefix=_chimara_marshal $< > $@

chimara-marshallers.h: chimara-marshallers.txt
	$(AM_V_GEN)glib-genmarshal --header --prefix=_chimara_marshal $< > $@

EXTRA_DIST = doc.c glkstart.c chimara-marshallers.txt
CLEANFILES =

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION
introspection_sources = \
	chimara-glk.c chimara-glk.h \
	chimara-if.c chimara-if.h

Chimara-1.0.gir: libchimara.la
Chimara_1_0_gir_INCLUDES = GObject-2.0 GLib-2.0 Gtk-3.0
Chimara_1_0_gir_CFLAGS = @CHIMARA_CFLAGS@ -I$(top_srcdir)
Chimara_1_0_gir_LIBS = libchimara.la
Chimara_1_0_gir_FILES = $(introspection_sources)
Chimara_1_0_gir_SCANNERFLAGS = \
	--warn-all \
	--c-include='libchimara/chimara-glk.h' \
	--c-include='libchimara/chimara-if.h'
Chimara_1_0_gir_EXPORT_PACKAGES = chimara
INTROSPECTION_GIRS += Chimara-1.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif
GITIGNOREFILES = Chimara-1.0.gir Chimara-1.0.typelib

vapidir = $(datadir)/vala/vapi
dist_vapi_DATA = chimara.vapi

if BUILDING_VAPI
chimara.vapi: $(INTROSPECTION_GIRS)
	$(AM_V_GEN)$(VAPIGEN) --library=chimara --pkg gtk+-3.0 Chimara-1.0.gir && \
	touch $@
endif

MAINTAINERCLEANFILES = chimara.vapi
-include $(top_srcdir)/git.mk
