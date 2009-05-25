# TODO: There are a lot more documentation files in the Nitfol distribution

PLUGIN_LIBTOOL_FLAGS=-module -avoid-version -export-symbols-regex "^glk"

GRAPHICS = no_graph.c no_graph.h
# GRAPHICS = graphics.c graphics.h
BLORB = blorb.c 
# BLORB = no_blorb.c no_blorb.h
SOUND = no_snd.c no_snd.h
# SOUND = sound.c sound.h

pkglib_LTLIBRARIES = nitfol.la
nitfol_la_SOURCES = automap.c automap.h binary.h debug.c debug.h decode.c \
	decode.h errmesg.c errmesg.h globals.c globals.h hash.c hash.h iff.c iff.h \
	infix.c infix.h inform.c inform.h init.c init.h io.c linkevil.h main.c \
	main.h nio.h nitfol.h objects.c objects.h op_call.c op_call.h op_jmp.c \
	op_jmp.h oplist.c oplist.h op_math.c op_math.h op_save.c op_save.h \
	op_table.c op_table.h op_v6.c op_v6.h portfunc.c portfunc.h quetzal.c \
	quetzal.h solve.c solve.h stack.c stack.h startunix.c struct.c struct.h \
	tokenise.c tokenise.h undo.c undo.h z_io.c z_io.h zscii.c zscii.h \
	$(GRAPHICS) \
	$(BLORB) \
	$(SOUND)
nitfol_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

EXTRA_DIST = crashme.inf test.inf 

nitfoldocdir = $(datadir)/doc/$(PACKAGE)/nitfol
dist_nitfoldoc_DATA = ChangeLog COPYING INSTALL README
