## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src po

chimaradocdir = ${prefix}/doc/chimara
chimaradoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS

EXTRA_DIST = $(chimaradoc_DATA) \
	intltool-extract.in intltool-merge.in intltool-update.in

# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

DISTCLEANFILES = intltool-extract intltool-merge intltool-update
