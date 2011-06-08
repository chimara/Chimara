pkglib_LTLIBRARIES = glulxe.la
glulxe_la_SOURCES = accel.c exec.c files.c funcs.c gestalt.c gestalt.h glkop.c \
    glulxe.h heap.c main.c opcodes.h operand.c osdepend.c profile.c search.c \
    serial.c string.c unixstrt.c vm.c float.c
glulxe_la_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libchimara -DOS_UNIX
glulxe_la_CFLAGS = -Wall -Wmissing-prototypes -Wstrict-prototypes -Wno-unused \
    $(AM_CFLAGS)
glulxe_la_LDFLAGS = -module $(PLUGIN_LIBTOOL_FLAGS)

#noinst_PROGRAMS = glulxdump
#glulxdump_SOURCES = glulxdump.c
#glulxdump_CPPFLAGS = -I$(top_srcdir)/src/libchimara
#glulxdump_LDADD = ../../libchimara/libchimara.la

dist_pkglibexec_SCRIPTS = profile-analyze.py

glulxedocdir = $(datadir)/doc/$(PACKAGE)/glulxe
dist_glulxedoc_DATA = README

-include $(top_srcdir)/git.mk
