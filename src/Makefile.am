## Process this file with automake to produce Makefile.in

AM_CFLAGS = -Wall

data_DATA = chimara.ui

noinst_PROGRAMS = test-chimara

test_chimara_SOURCES = main.c callbacks.c callbacks.h error.c error.h
test_chimara_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"
test_chimara_CFLAGS = @TEST_CFLAGS@ $(AM_CFLAGS)
test_chimara_LDADD = @TEST_LIBS@ -lchimara

lib_LTLIBRARIES = libchimara.la

libchimara_la_SOURCES = \
	abort.c abort.h \
	case.c \
	charset.c charset.h \
	chimara-glk.c chimara-glk.h chimara-glk-private.h \
	event.c event.h \
	fileref.c fileref.h \
	gestalt.c \
	glk.c glk.h \
	magic.c magic.h \
	input.c input.h \
	stream.c stream.h \
	strio.c \
	style.c \
	window.c window.h 
libchimara_la_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Chimara\"
libchimara_la_CFLAGS = @CHIMARA_CFLAGS@ $(AM_CFLAGS)
libchimara_la_LIBADD = @CHIMARA_LIBS@
libchimara_la_LDFLAGS = -no-undefined -export-symbols-regex "^chimara_glk_|^glk_"
libchimara_includedir = $(includedir)/chimara/chimara
libchimara_include_HEADERS = chimara-glk.h

PLUGIN_LDFLAGS = -module -shared -avoid-version -export-symbols-regex "^glk_main$$"
noinst_LTLIBRARIES = first.la model.la gridtest.la splittest.la
first_la_SOURCES = first.c
first_la_LDFLAGS = $(PLUGIN_LDFLAGS)
model_la_SOURCES = model.c
model_la_LDFLAGS = $(PLUGIN_LDFLAGS)
gridtest_la_SOURCES = gridtest.c
gridtest_la_LDFLAGS = $(PLUGIN_LDFLAGS)
splittest_la_SOURCES = splittest.c
splittest_la_LDFLAGS = $(PLUGIN_LDFLAGS)

CLEANFILES = chimara.ui

chimara.ui: chimara.glade
	gtk-builder-convert $< $@

EXTRA_DIST = doc.c chimara.glade
