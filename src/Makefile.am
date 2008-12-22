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

bin_PROGRAMS = chimara

# First line: User interface sources
# Second line: Glk program to run
chimara_SOURCES = \
	main.c callbacks.c callbacks.h error.c error.h \
	first.c \
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

chimara_LDADD = $(CHIMARA_LIBS)

CLEANFILES = chimara.ui

chimara.ui: chimara.glade
	gtk-builder-convert $< $@

EXTRA_DIST = chimara.glade
