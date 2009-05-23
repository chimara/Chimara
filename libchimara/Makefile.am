## Process this file with automake to produce Makefile.in

AM_CFLAGS = -Wall

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
	timer.c timer.h \
	window.c window.h \
	gi_blorb.c gi_blorb.h \
	resource.c resource.h \
	style.c style.h \
	glkstart.h
libchimara_la_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Chimara\"
libchimara_la_CFLAGS = @CHIMARA_CFLAGS@ $(AM_CFLAGS)
libchimara_la_LIBADD = @CHIMARA_LIBS@
libchimara_la_LDFLAGS = -no-undefined -export-symbols-regex "^(glk_|chimara_glk_|glkunix_|giblorb_)"
libchimara_includedir = $(includedir)/chimara/chimara
libchimara_include_HEADERS = chimara-glk.h

EXTRA_DIST = doc.c
