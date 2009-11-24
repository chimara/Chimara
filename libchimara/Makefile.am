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
	dispatch.c \
	event.c event.h \
	fileref.c fileref.h \
	garglk.c garglk.h \
	gestalt.c \
	gi_blorb.c gi_blorb.h \
	gi_dispa.c gi_dispa.h \
	glk.c glk.h \
	glkstart.h \
	glkunix.c glkunix.h \
	hyperlink.c hyperlink.h \
	image.c \
	init.c init.h \
	input.c input.h \
	magic.c magic.h \
	mouse.c \
	resource.c resource.h \
	schannel.c \
	stream.c stream.h \
	strio.c strio.h \
	style.c style.h \
	timer.c timer.h \
	window.c window.h 
libchimara_la_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Chimara\" \
	-DLOCALEDIR=\""$(datadir)/locale"\" \
	-DPLUGINDIR=\""$(pkglibdir)"\" \
	-DPLUGINSOURCEDIR=\""$(abs_builddir)/../interpreters"\" \
	-I$(top_srcdir)
libchimara_la_CFLAGS = @CHIMARA_CFLAGS@ $(AM_CFLAGS)
libchimara_la_LIBADD = @CHIMARA_LIBS@
libchimara_la_LDFLAGS = -version-info $(LT_VERSION_INFO) \
	-no-undefined \
	-export-symbols-regex "^(glk|chimara|glkunix|giblorb|gidispatch)_"
libchimara_includedir = $(includedir)/chimara/libchimara
libchimara_include_HEADERS = \
	chimara-glk.h \
	chimara-if.h \
	glk.h \
	glkstart.h \
	gi_blorb.h \
	garglk.h

BUILT_SOURCES = chimara-marshallers.c chimara-marshallers.h
chimara-marshallers.c: chimara-marshallers.txt
	glib-genmarshal --body --prefix=chimara_marshal $< > $@

chimara-marshallers.h: chimara-marshallers.txt
	glib-genmarshal --header --prefix=chimara_marshal $< > $@

EXTRA_DIST = doc.c glkstart.c chimara-marshallers.txt
