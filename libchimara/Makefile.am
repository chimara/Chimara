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
	garglk.c garglk.h \
	gestalt.c \
	gi_blorb.c gi_blorb.h \
	glk.c glk.h \
	glkstart.h \
	glkunix.c glkunix.h \
	input.c input.h \
	magic.c magic.h \
	mouse.c \
	resource.c resource.h \
	stream.c stream.h \
	strio.c \
	style.c style.h \
	timer.c timer.h \
	window.c window.h 
libchimara_la_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Chimara\"
libchimara_la_CFLAGS = @CHIMARA_CFLAGS@ $(AM_CFLAGS)
libchimara_la_LIBADD = @CHIMARA_LIBS@
libchimara_la_LDFLAGS = -no-undefined -export-symbols-regex "^(glk_|chimara_glk_|glkunix_|giblorb_)"
libchimara_includedir = $(includedir)/chimara/libchimara
libchimara_include_HEADERS = chimara-glk.h glk.h glkstart.h gi_blorb.h garglk.h

EXTRA_DIST = doc.c glkstart.c
