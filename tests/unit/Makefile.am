# The rpath is necessary for check_ too (see rant in ../Makefile.am)
TEST_PLUGIN_LIBTOOL_FLAGS = \
	-module \
	-shared \
	-avoid-version \
	-export-symbols-regex "^glk_main$$" \
	-rpath $(abs_builddir) \
	$(NULL)

# Set up include dirs so that #include "glk.h" works
AM_CPPFLAGS = -I$(top_srcdir)/libchimara $(CPPFLAGS)

check_LTLIBRARIES = datetime.la
datetime_la_SOURCES = datetime.c glkunit.c glkunit.h
datetime_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)

TESTS = datetime.la
TEST_EXTENSIONS = .la
LA_LOG_COMPILER = $(builddir)/../plugin-loader

CLEANFILES = \
	test-suite.log \
	$(TESTS) \
	$(TESTS:.la=.log) \
	$(TESTS:.la=.trs) \
	$(NULL)

-include $(top_srcdir)/git.mk
