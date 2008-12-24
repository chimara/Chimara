## Process this file with automake to produce Makefile.in

## Created by Anjuta

data_DATA = chimara.ui

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(CHIMARA_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = test-chimara

test_chimara_SOURCES = main.c callbacks.c callbacks.h error.c error.h
test_chimara_LDADD = $(TEST_LIBS) libchimara.la

lib_LTLIBRARIES = libchimara.la

libchimara_la_SOURCES = \
	abort.c abort.h \
	case.c \
	chimara-glk.c chimara-glk.h chimara-glk-private.h \
	event.c event.h \
	fileref.c fileref.h \
	gestalt.c \
	glk.c glk.h \
	input.c input.h \
	stream.c stream.h \
	strio.c \
	style.c \
	window.c window.h
libchimara_la_LIBADD = $(CHIMARA_LIBS)
libchimara_la_LDFLAGS = -no-undefined -export-symbols-regex "^chimara_glk_.*|^glk_.*"
libchimara_includedir = $(includedir)/chimara/chimara
libchimara_include_HEADERS = chimara-glk.h

pkglib_LTLIBRARIES = first.la model.la gridtest.la

first_la_SOURCES = first.c
first_la_LIBADD = libchimara.la
first_la_LDFLAGS = -module -avoid-version

model_la_SOURCES = model.c
model_la_LIBADD = libchimara.la
model_la_LDFLAGS = -module -avoid-version

gridtest_la_SOURCES = gridtest.c
gridtest_la_LIBADD = libchimara.la
gridtest_la_LDFLAGS = -module -avoid-version

CLEANFILES = chimara.ui

chimara.ui: chimara.glade
	gtk-builder-convert $< $@

EXTRA_DIST = chimara.glade
