# Automatically generate version.h
MAJOR = 1
MINOR = 2
PATCH = 9
version.h: Makefile.am
	$(AM_V_GEN)echo "// Automatically generated file -- do not edit!" > version.h
	$(AM_V_at)echo "#define GIT_MAJOR" $(MAJOR) >> version.h
	$(AM_V_at)echo "#define GIT_MINOR" $(MINOR) >> version.h
	$(AM_V_at)echo "#define GIT_PATCH" $(PATCH) >> version.h

pkglib_LTLIBRARIES = git.la
BUILT_SOURCES = version.h
git_la_SOURCES = version.h git.h config.h compiler.h memory.h opcodes.h \
    labels.inc compiler.c gestalt.c git.c git_unix.c glkop.c heap.c memory.c \
    opcodes.c operands.c peephole.c savefile.c saveundo.c search.c terp.c \
    accel.c
git_la_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libchimara
git_la_CFLAGS = -DCHIMARA_EXTENSIONS -DUSE_INLINE $(AM_CFLAGS)
git_la_LDFLAGS = -module $(PLUGIN_LIBTOOL_FLAGS)

gitdocdir = $(datadir)/doc/$(PACKAGE)/git
dist_gitdoc_DATA = README.txt

CLEANFILES = version.h test/*.tmp

#TESTS = test/test.sh \
#	test/Alabaster.gblorb test/Alabaster.walk test/Alabaster.golden
#
#test: git
#	sh test/test.sh

# Memory mapping (-DUSE_MMAP) doesn't seem to work... FIXME

# Best settings for GCC 2.95. This generates faster code than
# GCC 3, so you should use this setup if possible.
#CC = gcc -Wall -O3
#OPTIONS = -DUSE_DIRECT_THREADING -DUSE_MMAP -DUSE_INLINE
#
# Best settings for GCC 3. The optimiser in this version of GCC
# is somewhat broken, so we can't use USE_DIRECT_THREADING.
#CC = gcc -Wall -O3
#OPTIONS = -DUSE_MMAP -DUSE_INLINE
#
# Mac OS X (PowerPC) settings.
#CC = gcc2 -Wall -O3 -no-cpp-precomp
#OPTIONS = -DUSE_DIRECT_THREADING -DUSE_BIG_ENDIAN_UNALIGNED -DUSE_MMAP -DUSE_INLINE

-include $(top_srcdir)/git.mk
