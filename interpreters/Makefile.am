# All plugins

plugindir = $(pkglibdir)/plugins
plugin_LTLIBRARIES = libbocfel.la libfrotz.la libgit.la libglulxe.la \
	libnitfol.la
CLEANFILES =

# Bocfel

libbocfel_la_SOURCES = bocfel/blorb.c bocfel/blorb.h bocfel/branch.c \
	bocfel/branch.h bocfel/dict.c bocfel/dict.h bocfel/glkstart.c bocfel/iff.c \
	bocfel/iff.h bocfel/io.c bocfel/io.h bocfel/math.c bocfel/math.h \
	bocfel/memory.c bocfel/memory.h bocfel/objects.c bocfel/objects.h \
	bocfel/osdep.c bocfel/osdep.h bocfel/process.c bocfel/process.h \
	bocfel/random.c bocfel/random.h bocfel/screen.c bocfel/screen.h \
	bocfel/stack.c bocfel/stack.h bocfel/table.c bocfel/table.h \
	bocfel/unicode.c bocfel/unicode.h bocfel/util.c bocfel/util.h \
	bocfel/zoom.c bocfel/zoom.h bocfel/zterp.c bocfel/zterp.h \
	chimara-bocfel-plugin.c chimara-bocfel-plugin.h
libbocfel_la_CPPFLAGS = -DZTERP_GLK -DZTERP_UNIX \
	-I$(top_srcdir) -I$(top_srcdir)/libchimara
libbocfel_la_CFLAGS = -std=c99 $(PLUGIN_CFLAGS) $(AM_CFLAGS)
libbocfel_la_LDFLAGS = -module $(PLUGIN_LIBS) $(PLUGIN_LIBTOOL_FLAGS)

# Frotz

libfrotz_la_SOURCES = frotz/buffer.c frotz/err.c frotz/fastmem.c frotz/files.c \
	frotz/input.c frotz/main.c frotz/math.c frotz/object.c frotz/process.c \
	frotz/quetzal.c frotz/random.c frotz/redirect.c frotz/sound.c \
	frotz/stream.c frotz/table.c frotz/text.c frotz/variable.c \
	frotz/glkscreen.c frotz/glkmisc.c frotz/frotz.h frotz/glkfrotz.h \
	frotz/glkio.h chimara-frotz-plugin.c chimara-frotz-plugin.h
libfrotz_la_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libchimara
libfrotz_la_CFLAGS = -Wno-pointer-sign $(PLUGIN_CFLAGS) $(AM_CFLAGS)
libfrotz_la_LDFLAGS = -module $(PLUGIN_LIBS) $(PLUGIN_LIBTOOL_FLAGS)
frotzdatadir = $(plugindir)/frotz
dist_frotzdata_DATA = chimara-frotz-plugin.glade

# Git

libgit_la_SOURCES = git/version.h git/git.h git/config.h git/compiler.h \
	git/memory.h git/opcodes.h git/labels.inc git/compiler.c git/gestalt.c \
	git/git.c git/git_unix.c git/glkop.c git/heap.c git/memory.c git/opcodes.c \
	git/operands.c git/peephole.c git/savefile.c git/saveundo.c git/search.c \
	git/terp.c git/accel.c chimara-git-plugin.c chimara-git-plugin.h
libgit_la_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libchimara
libgit_la_CFLAGS = -DCHIMARA_EXTENSIONS -DUSE_INLINE \
	$(PLUGIN_CFLAGS) $(AM_CFLAGS)
libgit_la_LDFLAGS = -module $(PLUGIN_LIBS) $(PLUGIN_LIBTOOL_FLAGS)
# Memory mapping (-DUSE_MMAP) doesn't seem to work... FIXME

CLEANFILES += git/test/*.tmp

# Glulxe

libglulxe_la_SOURCES = glulxe/accel.c glulxe/exec.c glulxe/files.c \
	glulxe/funcs.c glulxe/gestalt.c glulxe/gestalt.h glulxe/glkop.c \
	glulxe/glulxe.h glulxe/heap.c glulxe/main.c glulxe/opcodes.h \
	glulxe/operand.c glulxe/osdepend.c glulxe/profile.c glulxe/search.c \
	glulxe/serial.c glulxe/string.c glulxe/unixstrt.c glulxe/vm.c \
	glulxe/float.c chimara-glulxe-plugin.c chimara-glulxe-plugin.h
libglulxe_la_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libchimara -DOS_UNIX
libglulxe_la_CFLAGS = -Wall -Wmissing-prototypes -Wstrict-prototypes \
	-Wno-unused $(PLUGIN_CFLAGS) $(AM_CFLAGS)
libglulxe_la_LDFLAGS = -module $(PLUGIN_LIBS) $(PLUGIN_LIBTOOL_FLAGS)

#noinst_PROGRAMS = glulxdump
#glulxdump_SOURCES = glulxe/glulxdump.c
#glulxdump_CPPFLAGS = -I$(top_srcdir)/src/libchimara
#glulxdump_LDADD = ../libchimara/libchimara.la

dist_pkglibexec_SCRIPTS = glulxe/profile-analyze.py

# Nitfol

NITFOL_GRAPHICS = nitfol/graphics.c nitfol/graphics.h
NITFOL_BLORB = nitfol/blorb.c
NITFOL_SOUND = nitfol/sound.c nitfol/sound.h
# NITFOL_GRAPHICS = nitfol/no_graph.c nitfol/no_graph.h
# NITFOL_BLORB = nitfol/no_blorb.c
# NITFOL_SOUND = nitfol/no_snd.c nitfol/no_snd.h
# no_blorb.h is apparently the header file for both blorb.c and no_blorb.c

dist_noinst_SCRIPTS = nitfol/copying.awk nitfol/opt2glkc.pl nitfol/y2help.pl

libnitfol_la_SOURCES = nitfol/automap.c nitfol/automap.h nitfol/binary.h \
	nitfol/copying.h nitfol/debug.c nitfol/debug.h nitfol/decode.c \
	nitfol/decode.h nitfol/errmesg.c nitfol/errmesg.h nitfol/globals.c \
	nitfol/globals.h nitfol/hash.c nitfol/hash.h nitfol/iff.c nitfol/iff.h \
	nitfol/infix.c nitfol/infix.h nitfol/inform.y nitfol/inform.h \
	nitfol/init.c nitfol/init.h nitfol/io.c nitfol/nio.h nitfol/linkevil.h \
	nitfol/main.c nitfol/main.h nitfol/nitfol.h nitfol/no_blorb.h \
	nitfol/objects.c nitfol/objects.h nitfol/op_call.c nitfol/op_call.h \
	nitfol/op_jmp.c nitfol/op_jmp.h nitfol/oplist.c nitfol/oplist.h \
	nitfol/op_math.c nitfol/op_math.h nitfol/op_save.c nitfol/op_save.h \
	nitfol/op_table.c nitfol/op_table.h nitfol/op_v6.c nitfol/op_v6.h \
	nitfol/portfunc.c nitfol/portfunc.h nitfol/quetzal.c nitfol/quetzal.h \
	nitfol/solve.c nitfol/solve.h nitfol/stack.c nitfol/stack.h \
	nitfol/struct.c nitfol/struct.h nitfol/tokenise.c nitfol/tokenise.h \
	nitfol/undo.c nitfol/undo.h nitfol/z_io.c nitfol/z_io.h nitfol/zscii.c \
	nitfol/zscii.h \
	$(NITFOL_GRAPHICS) $(NITFOL_BLORB) $(NITFOL_SOUND) \
	chimara-nitfol-plugin.c chimara-nitfol-plugin.h
nodist_libnitfol_la_SOURCES = nitfol/copying.c nitfol/dbg_help.h \
	nitfol/startunix.c
libnitfol_la_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libchimara \
	-I$(srcdir)/nitfol -DSMART_TOKENISER -DUSE_INLINE
libnitfol_la_CFLAGS = $(PLUGIN_CFLAGS) $(AM_CFLAGS)
libnitfol_la_LDFLAGS = -module $(PLUGIN_LIBS) $(PLUGIN_LIBTOOL_FLAGS)

info_TEXINFOS = nitfol/nitfol.texi
nitfol_TEXINFOS = nitfol/dbg_help.texi nitfol/options.texi
# man_MANS = nitfol/nitfol.6

BUILT_SOURCES = nitfol/copying.c nitfol/dbg_help.h nitfol/startunix.c \
	nitfol/dbg_help.texi nitfol/options.texi
CLEANFILES += $(BUILT_SOURCES) nitfol/nitfol.6
DISTCLEANFILES = nitfol/inform.c nitfol/nitfol.info
MAINTAINERCLEANFILES = texinfo.tex

nitfol/dbg_help.h: nitfol/inform.y nitfol/y2help.pl
	$(AM_V_GEN)cd nitfol && \
	$(PERL) $(abs_srcdir)/nitfol/y2help.pl $(abs_srcdir)/nitfol/inform.y

nitfol/dbg_help.texi: nitfol/inform.y nitfol/y2help.pl
	$(AM_V_GEN)cd nitfol && \
	$(PERL) $(abs_srcdir)/nitfol/y2help.pl $(abs_srcdir)/nitfol/inform.y

nitfol/copying.c: nitfol/COPYING nitfol/copying.awk
	$(AM_V_GEN)$(AWK) -f $(srcdir)/nitfol/copying.awk <$(srcdir)/nitfol/COPYING >$@

# nitfol/nitfol.6: nitfol/nitfol.opt nitfol/opt2glkc.pl
# 	$(AM_V_GEN)cd nitfol && \
#	$(PERL) $(abs_srcdir)/nitfol/opt2glkc.pl -man $(abs_srcdir)/nitfol/nitfol.opt

nitfol/options.texi: nitfol/nitfol.opt nitfol/opt2glkc.pl
	$(AM_V_GEN)cd nitfol && \
	$(PERL) $(abs_srcdir)/nitfol/opt2glkc.pl -info $(abs_srcdir)/nitfol/nitfol.opt

nitfol/startunix.c: nitfol/nitfol.opt nitfol/opt2glkc.pl
	$(AM_V_GEN)cd nitfol && \
	$(PERL) $(abs_srcdir)/nitfol/opt2glkc.pl -unix $(abs_srcdir)/nitfol/nitfol.opt

EXTRA_DIST = nitfol/nitfol.opt nitfol/crashme.inf nitfol/test.inf

# Plugin files

dist_plugin_DATA = bocfel.plugin frotz.plugin git.plugin glulxe.plugin \
	nitfol.plugin

# Documentation

pkgdocdir = $(datadir)/doc/$(PACKAGE)
nobase_dist_pkgdoc_DATA = bocfel/BUILDING bocfel/COPYING.GPLv2 \
	bocfel/COPYING.GPLv3 bocfel/README \
	frotz/AUTHORS frotz/COPYING frotz/README frotz/TODO \
	git/README.txt \
	glulxe/README \
	nitfol/ChangeLog nitfol/COPYING nitfol/INSTALL nitfol/README

-include $(top_srcdir)/git.mk
