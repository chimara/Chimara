AM_CFLAGS = -Wall
AM_CPPFLAGS = -I$(top_srcdir)

# The rpath is necessary because stoopid libtool won't build a shared library
# if it's noinst_, because what POSSIBLE reason could you have to do that?
TEST_PLUGIN_LIBTOOL_FLAGS = \
	-module \
	-shared \
	-avoid-version \
	-export-symbols-regex "^glk_main$$" \
	-rpath $(abs_builddir)

noinst_PROGRAMS = test-multisession glulxercise plugin-loader test-close

test_multisession_SOURCES = test-multisession.c
test_multisession_CFLAGS = @TEST_CFLAGS@ $(AM_CFLAGS)
test_multisession_LDADD = @TEST_LIBS@ $(top_builddir)/libchimara/libchimara.la

glulxercise_SOURCES = glulxercise.c
glulxercise_CPPFLAGS = $(AM_CPPFLAGS) -DPACKAGE_SRC_DIR=\""$(srcdir)"\"
glulxercise_CFLAGS = @TEST_CFLAGS@ $(AM_CFLAGS) 
glulxercise_LDADD = @TEST_LIBS@ $(top_builddir)/libchimara/libchimara.la

plugin_loader_SOURCES = plugin-loader.c
plugin_loader_CFLAGS = @TEST_CFLAGS@ $(AM_CFLAGS)
plugin_loader_LDADD = @TEST_LIBS@ $(top_builddir)/libchimara/libchimara.la

test_close_SOURCES = test-close.c
test_close_CFLAGS = @TEST_CFLAGS@ $(AM_CFLAGS)
test_close_LDADD = @TEST_LIBS@ $(top_builddir)/libchimara/libchimara.la

noinst_LTLIBRARIES = first.la model.la gridtest.la splittest.la multiwin.la \
	styletest.la soundtest.la test-userstyle.la

first_la_SOURCES = first.c
first_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)

model_la_SOURCES = model.c
model_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)

gridtest_la_SOURCES = gridtest.c
gridtest_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)

splittest_la_SOURCES = splittest.c
splittest_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)

multiwin_la_SOURCES = multiwin.c
multiwin_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)

styletest_la_SOURCES = styletest.c
styletest_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)

test_userstyle_la_SOURCES = test-userstyle.c
test_userstyle_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)

soundtest_la_SOURCES = soundtest.c
soundtest_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)


-include $(top_srcdir)/git.mk
