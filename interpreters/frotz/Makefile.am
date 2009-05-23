PLUGIN_LIBTOOL_FLAGS=-module -avoid-version -export-symbols-regex "^glk_main$$"

pkglib_LTLIBRARIES = frotz.la
frotz_la_SOURCES = buffer.c err.c fastmem.c files.c input.c main.c math.c \
	object.c process.c quetzal.c random.c redirect.c sound.c stream.c table.c \
	text.c variable.c glkscreen.c glkmisc.c frotz.h glkfrotz.h glkio.h setup.h
frotz_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

frotzdocdir = $(datadir)/doc/$(PACKAGE)/frotz
dist_frotzdoc_DATA = AUTHORS COPYING README TODO

