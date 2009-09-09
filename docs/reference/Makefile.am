### Makefile.am for Gtk-Doc directory #########################################
# Adapted from examples/Makefile.am in Gtk-Doc distribution

# We require automake 1.6 at least.
AUTOMAKE_OPTIONS = 1.6

# The name of the module, e.g. 'glib'.
DOC_MODULE = chimara

# Uncomment for versioned docs and specify the version of the module, e.g. '2'.
# DOC_MODULE_VERSION = 2

# The top-level SGML file. You can change this if you want to.
DOC_MAIN_SGML_FILE = $(DOC_MODULE)-docs.sgml

# The directory containing the source code. Relative to $(srcdir).
# gtk-doc will search all .c & .h files beneath here for inline comments
# documenting the functions and macros.
DOC_SOURCE_DIR = ../../libchimara

# Extra options to pass to gtkdoc-scangobj. Not normally needed.
SCANGOBJ_OPTIONS = 

# Extra options to supply to gtkdoc-scan.
# DO NOT use --rebuild-types, because it will think glk_window_get_type()
# is a GObject type. That makes it segfault.
SCAN_OPTIONS = 

# Extra options to supply to gtkdoc-mkdb.
MKDB_OPTIONS = --sgml-mode --output-format=xml

# Extra options to supply to gtkdoc-mktmpl
MKTMPL_OPTIONS = 

# Extra options to supply to gtkdoc-mkhtml
MKHTML_OPTIONS = 

# Extra options to supply to gtkdoc-fixref. Not normally needed.
FIXXREF_OPTIONS = 

# Used for dependencies. The docs will be rebuilt if any of these change.
HFILE_GLOB = $(top_srcdir)/libchimara/*.h
CFILE_GLOB = $(top_srcdir)/libchimara/*.c

# Extra headers to include when scanning, which are not under DOC_SOURCE_DIR
EXTRA_HFILES = 

# Header files to ignore when scanning. Use base file name, no paths
IGNORE_HFILES = chimara-glk-private.h \
	abort.h charset.h event.h input.h stream.h window.h

# Images to copy into HTML directory.
HTML_IMAGES = \
	$(srcdir)/images/fig1.png \
	$(srcdir)/images/fig2.png \
	$(srcdir)/images/fig3a.png \
	$(srcdir)/images/fig3b.png \
	$(srcdir)/images/fig3c.png \
	$(srcdir)/images/fig4a.png \
	$(srcdir)/images/fig4b.png \
	$(srcdir)/images/fig4c.png \
	$(srcdir)/images/fig5-7a.png \
	$(srcdir)/images/fig6.png \
	$(srcdir)/images/fig7b.png \
	$(srcdir)/images/fig7c.png \
	$(srcdir)/images/fig7d.png \
	$(srcdir)/images/fig7e.png \
	$(srcdir)/images/fig8a.png \
	$(srcdir)/images/fig8b.png \
	$(srcdir)/images/fig8c.png \
	$(srcdir)/images/fig9.png \
	$(srcdir)/images/fig10.png \
	$(srcdir)/images/fig11.png

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE).
content_files = version.xml \
	glk-front-matter.sgml \
	glk-introduction.sgml \
	glk-main-function.sgml \
	glk-api-conventions.sgml \
	glk-character-encoding.sgml \
	glk-output.sgml \
	glk-line-input.sgml \
	glk-windows.sgml \
	glk-window-arrangement.sgml \
	glk-display-style.sgml \
	glk-other-events.sgml

# SGML files where gtk-doc abbreviations (#GtkWidget) are expanded
# These files must be listed here *and* in content_files
expand_content_files = \
	glk-main-function.sgml \
	glk-api-conventions.sgml \
	glk-output.sgml \
	glk-line-input.sgml \
	glk-window-arrangement.sgml \
	glk-display-style.sgml \
	glk-other-events.sgml

# CFLAGS and LDFLAGS for compiling gtkdoc-scangobj with your library.
# Only needed if you are using gtkdoc-scangobj to dynamically query widget
# signals and properties.
GTKDOC_CFLAGS = -I$(top_srcdir) $(CHIMARA_CFLAGS)
GTKDOC_LIBS =  $(top_builddir)/libchimara/libchimara.la

# This includes the standard gtk-doc make rules, copied by gtkdocize.
include $(top_srcdir)/gtk-doc.make

# Other files to distribute
EXTRA_DIST += version.xml.in

# Files not to distribute
# for --rebuild-types in $(SCAN_OPTIONS), e.g. $(DOC_MODULE).types
# for --rebuild-sections in $(SCAN_OPTIONS), e.g. $(DOC_MODULE)-sections.txt
DISTCLEANFILES = $(DOC_MODULE)-overrides.txt

# Uncomment this if you want your docs-status tested during 'make check'
# Comment this while not all symbols are documented, otherwise distcheck fails
#if ENABLE_GTK_DOC
#TESTS_ENVIRONMENT = cd $(srcdir) &&
#TESTS = $(GTKDOC_CHECK)
#endif
