# TODO: There are a lot more documentation files in the Nitfol distribution

PLUGIN_LIBTOOL_FLAGS=-module -avoid-version -export-symbols-regex "^glk_main$$"

GRAPHICS = no_graph.c
# GRAPHICS = graphics.c
BLORB = blorb.c
# BLORB = no_blorb.c
SOUND = no_snd.c
# SOUND = sound.c

pkglib_LTLIBRARIES = nitfol.la
nitfol_la_SOURCES = automap.c solve.c infix.c debug.c inform.c quetzal.c \
	undo.c op_call.c decode.c errmesg.c globals.c iff.c init.c main.c io.c \
	z_io.c op_jmp.c op_math.c op_save.c op_table.c op_v6.c oplist.c stack.c \
	zscii.c tokenise.c struct.c objects.c portfunc.c hash.c \
	$(GRAPHICS) \
	$(BLORB) \
	$(SOUND)
nitfol_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

nitfoldocdir = $(datadir)/doc/$(PACKAGE)/nitfol
dist_nitfoldoc_DATA = ChangeLog COPYING INSTALL README
