AM_CFLAGS = -Wall
AM_CPPFLAGS = -I$(top_srcdir)

dist_data_DATA = chimara.ui chimara.menus glulxercise.ui

noinst_PROGRAMS = test-chimara test-multisession glulxercise

test_chimara_SOURCES = main.c callbacks.c error.c error.h
test_chimara_CPPFLAGS = $(AM_CPPFLAGS) \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"
test_chimara_CFLAGS = @TEST_CFLAGS@ $(AM_CFLAGS)
test_chimara_LDADD = @TEST_LIBS@ $(top_builddir)/libchimara/libchimara.la

test_multisession_SOURCES = test-multisession.c
test_multisession_CFLAGS = @TEST_CFLAGS@ $(AM_CFLAGS)
test_multisession_LDADD = @TEST_LIBS@ $(top_builddir)/libchimara/libchimara.la

glulxercise_SOURCES = glulxercise.c error.c error.h
glulxercise_CFLAGS = @TEST_CFLAGS@ $(AM_CFLAGS)
glulxercise_LDADD = @TEST_LIBS@ $(top_builddir)/libchimara/libchimara.la

pkglib_LTLIBRARIES = first.la model.la gridtest.la splittest.la multiwin.la
PLUGIN_LIBTOOL_FLAGS=-module -avoid-version -export-symbols-regex "^glk_main$$"

first_la_SOURCES = first.c
first_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

model_la_SOURCES = model.c
model_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

gridtest_la_SOURCES = gridtest.c
gridtest_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

splittest_la_SOURCES = splittest.c
splittest_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)

multiwin_la_SOURCES = multiwin.c
multiwin_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
