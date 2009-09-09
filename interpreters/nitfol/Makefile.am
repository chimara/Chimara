PLUGIN_LIBTOOL_FLAGS=-module -avoid-version -export-symbols-regex "^glk"

GRAPHICS = no_graph.c no_graph.h
# GRAPHICS = graphics.c graphics.h
BLORB = blorb.c 
# BLORB = no_blorb.c
SOUND = no_snd.c no_snd.h
# SOUND = sound.c sound.h

dist_noinst_SCRIPTS = copying.awk opt2glkc.pl y2help.pl

pkglib_LTLIBRARIES = nitfol.la
nitfol_la_SOURCES = automap.c automap.h binary.h copying.h debug.c debug.h \
	decode.c decode.h errmesg.c errmesg.h globals.c globals.h hash.c hash.h \
	iff.c iff.h infix.c infix.h inform.y inform.h init.c init.h io.c \
	linkevil.h main.c main.h nio.h nitfol.h no_blorb.h objects.c objects.h \
	op_call.c op_call.h op_jmp.c op_jmp.h oplist.c oplist.h op_math.c \
	op_math.h op_save.c op_save.h op_table.c op_table.h op_v6.c op_v6.h \
	portfunc.c portfunc.h quetzal.c quetzal.h solve.c solve.h stack.c stack.h \
	struct.c struct.h tokenise.c tokenise.h undo.c undo.h z_io.c z_io.h \
	zscii.c zscii.h \
	$(GRAPHICS) \
	$(BLORB) \
	$(SOUND)
nodist_nitfol_la_SOURCES = copying.c dbg_help.h startunix.c
nitfol_la_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libchimara \
	-DSMART_TOKENISER -DUSE_INLINE
nitfol_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

info_TEXINFOS = nitfol.texi
nitfol_TEXINFOS = dbg_help.texi options.texi
# man_MANS = nitfol.6

BUILT_SOURCES = copying.c dbg_help.h startunix.c
CLEANFILES = $(BUILT_SOURCES) dbg_help.texi nitfol.6 options.texi 

dbg_help.h: inform.y y2help.pl
	$(PERL) $(srcdir)/y2help.pl $(srcdir)/inform.y

dbg_help.texi: inform.y y2help.pl
	$(PERL) $(srcdir)/y2help.pl $(srcdir)/inform.y

copying.c: COPYING copying.awk
	$(AWK) --file $(srcdir)/copying.awk <$(srcdir)/COPYING >copying.c

# nitfol.6: nitfol.opt opt2glkc.pl
# 	$(PERL) $(srcdir)/opt2glkc.pl -man $(srcdir)/nitfol.opt

options.texi: nitfol.opt opt2glkc.pl
	$(PERL) $(srcdir)/opt2glkc.pl -info $(srcdir)/nitfol.opt

startunix.c: nitfol.opt opt2glkc.pl
	$(PERL) $(srcdir)/opt2glkc.pl -unix $(srcdir)/nitfol.opt

EXTRA_DIST = nitfol.opt crashme.inf test.inf 

nitfoldocdir = $(datadir)/doc/$(PACKAGE)/nitfol
dist_nitfoldoc_DATA = ChangeLog COPYING INSTALL README
