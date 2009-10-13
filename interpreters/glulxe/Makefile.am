pkglib_LTLIBRARIES = glulxe.la
glulxe_la_SOURCES = accel.c exec.c files.c funcs.c gestalt.c gestalt.h glkop.c \
    glulxe.h heap.c main.c opcodes.h operand.c osdepend.c profile.c search.c \
    serial.c string.c unixstrt.c vm.c 
glulxe_la_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libchimara
glulxe_la_CFLAGS = -Wall -Wmissing-prototypes -Wstrict-prototypes -Wno-unused \
    -DOS_UNIX $(AM_CFLAGS)
glulxe_la_LDFLAGS = -module $(PLUGIN_LIBTOOL_FLAGS)

glulxedocdir = $(datadir)/doc/$(PACKAGE)/glulxe
dist_glulxedoc_DATA = README
