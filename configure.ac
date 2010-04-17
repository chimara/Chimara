# configure.ac with sensible comments

### INITIALIZATION ############################################################

# Initialize Autoconf 
# Args: (human-readable package name, package version, bug report address, 
# tarballname)
AC_INIT([chimara], [0.1])
# Sanity check to make sure we are running Autoconf from the right directory
AC_CONFIG_SRCDIR(libchimara/chimara-glk.c)
# Put m4 macros in their own directory
AC_CONFIG_MACRO_DIR(m4)
# Initialize Automake
AM_INIT_AUTOMAKE([-Wall])
# Configure with --enable-silent-rules to cut down on clutter
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

### LIBRARY VERSIONING ########################################################
# Only update immediately before a public release

CHIMARA_CURRENT=0  # Increment if interface changes
CHIMARA_REVISION=0 # Increment if source changes; set 0 if interface changes
CHIMARA_AGE=0 # Increment if interfaces added; set 0 if removed
LT_VERSION_INFO="$CHIMARA_CURRENT:$CHIMARA_REVISION:$CHIMARA_AGE"
AC_SUBST(LT_VERSION_INFO)

### REQUIREMENTS ##############################################################
GTK_REQUIRED_VERSION=2.12
GLIB_REQUIRED_VERSION=2.16
GTK_DOC_REQUIRED_VERSION=1.9
AC_SUBST(GTK_REQUIRED_VERSION)
AC_SUBST(GLIB_REQUIRED_VERSION)
AC_SUBST(GTK_DOC_REQUIRED_VERSION)

### DECLARE COMPILERS #########################################################

AC_USE_SYSTEM_EXTENSIONS     # Define _GNU_SOURCE if using GCC
AC_PROG_CC                   # C compiler
AM_PROG_CC_C_O               # Automake requires this for per-target CFLAGS
AC_C_INLINE                  # Define inline keyword 
AC_PROG_YACC                 # Building nitfol requires yacc

### DECLARE PROGRAMS ##########################################################

AC_PROG_INSTALL              # Install
m4_defun([_LT_AC_LANG_CXX_CONFIG], [:]) # Disable unnecessary Libtool checks
m4_defun([_LT_AC_LANG_F77_CONFIG], [:]) # to save time (1.5 only)
LT_INIT                      # Libtool 2.2.6 and up
#LT_INIT([dlopen])           # Should call it this way, but
#LT_PREREQ([2.2.6])          # ...goddamn Debian still has 1.5
AC_PROG_LIBTOOL              # Old way of declaring Libtool
AM_GNU_GETTEXT([external])   # Gettext, link to system libintl
IT_PROG_INTLTOOL([0.40.0])   # Intltool
PKG_PROG_PKG_CONFIG          # pkg_config
GTK_DOC_CHECK($GTK_DOC_REQUIRED_VERSION) 
# Other utilities used in this package's various make scripts
AC_PROG_AWK
AC_PATH_PROG([PERL], [perl]) # Perl
AC_PATH_PROG([TEST], [test]) # Test
AC_PATH_PROG([ECHO], [echo]) # Echo

### TYPES #####################################################################

AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT32_T

### INTERNATIONALIZATION ######################################################

AM_GNU_GETTEXT_VERSION([0.17])
GETTEXT_PACKAGE=chimara
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

### ILIAD #####################################################################
AC_ARG_ENABLE([iliad],
  [AS_HELP_STRING([--enable-iliad],[Compiles Chimara for the iLiad])],
  [AS_CASE([${enableval}],
	   [yes], [iliad=true],
	   [no], [iliad=false],
	   [AC_MSG_ERROR([bad value ${enableval} for --enable-iliad])]
   )],
   [iliad=false]
)

AM_CONDITIONAL([TARGET_ILIAD], [$TEST "x$iliad" = xtrue])

### RPM CONFIGURATION ##########################################################
# --enable-rpm requires rpm and rpmbuild
AC_PATH_PROG([RPMBUILD], [rpmbuild], [notfound])
AC_ARG_ENABLE([rpm],
	[AS_HELP_STRING([--enable-rpm],[Configure for building RPM package [no] (requires rpm and rpmbuild)])],
	[AS_CASE([${enableval}],
		[yes], [AS_IF(
			[$TEST "x$RPMBUILD" = xnotfound],
				[AC_MSG_ERROR([rpmbuild is required for --enable-rpm])],
			[rpm=true])],
		[no],  [rpm=false],
		[AC_MSG_ERROR([bad value ${enableval} for --enable-rpm])]
	)],
	[rpm=false]
)
AM_CONDITIONAL([BUILDING_RPM], [$TEST "x$rpm" = xtrue])

### CHECK FOR LIBRARIES #######################################################

# Libraries needed to build Chimara library
PKG_CHECK_MODULES([CHIMARA], [
	glib-2.0 >= $GLIB_REQUIRED_VERSION
	gtk+-2.0 >= $GTK_REQUIRED_VERSION
	gthread-2.0 
	gmodule-2.0
	pango
])
CHIMARA_LIBS="$CHIMARA_LIBS -lm"
AC_SUBST(CHIMARA_LIBS)
# Libraries needed to build test programs
PKG_CHECK_MODULES([TEST], [
	gtk+-2.0 >= $GTK_REQUIRED_VERSION
	gmodule-2.0 >= $GLIB_REQUIRED_VERSION
])

# Plugin flags; include '-module' in each Makefile.am, because AC_SUBSTed
# variables are black boxes to Automake, so it has to know about it being a
# module in the makefile itself.
PLUGIN_LIBTOOL_FLAGS='-avoid-version -shared -export-symbols-regex "^glk"'
AC_SUBST(PLUGIN_LIBTOOL_FLAGS)

### OUTPUT ####################################################################

# Output platform-specific definitions to config.h
AC_CONFIG_HEADERS([config.h])
# List of other files for Autoconf to output
AC_CONFIG_FILES([
Makefile
chimara.pc
chimara-plugin.pc
chimara.spec
libchimara/Makefile
interpreters/Makefile
interpreters/frotz/Makefile
interpreters/nitfol/Makefile
interpreters/glulxe/Makefile
interpreters/git/Makefile
tests/Makefile
player/Makefile
docs/Makefile
docs/reference/Makefile
docs/reference/version.xml
docs/reference/build-selector-table.pl
po/Makefile.in
])

# Do it
AC_OUTPUT

