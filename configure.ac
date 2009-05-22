# configure.ac with sensible comments

### INITIALIZATION ############################################################

# Initialize Autoconf 
# Args: (human-readable package name, package version, bug report address, 
# tarballname)
AC_INIT([chimara], [0.1])
# Sanity check to make sure we are running Autoconf from the right directory
AC_CONFIG_SRCDIR(src/chimara-glk.c)
# Initialize Automake
AM_INIT_AUTOMAKE([-Wall])

### DECLARE PROGRAMS ##########################################################

AC_PROG_CC                 # C compiler
AM_PROG_CC_C_O             # Automake requires this for per-target CFLAGS
AC_PROG_INSTALL            # Install
LT_INIT                    # Libtool 2.2.6 and up
#LT_INIT([dlopen])         # Should call it this way, but
#LT_PREREQ([2.2.6])        # ...goddamn Fedora 10 still has 1.5
AC_PROG_LIBTOOL            # Old way of declaring Libtool
AM_GLIB_GNU_GETTEXT        # Use GLib gettext instead of standard
IT_PROG_INTLTOOL([0.35.0]) # Intltool
PKG_PROG_PKG_CONFIG        # pkg_config
GTK_DOC_CHECK(1.9)         # Gtk-Doc

### TYPES #####################################################################

AC_TYPE_INT32_T
AC_TYPE_UINT32_T

### INTERNATIONALIZATION ######################################################

GETTEXT_PACKAGE=chimara
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

### CHECK FOR LIBRARIES #######################################################

GTK_REQUIRED_VERSION=2.8
GLIB_REQUIRED_VERSION=2.6

AC_SUBST(GTK_REQUIRED_VERSION)
AC_SUBST(GLIB_REQUIRED_VERSION)

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
	gmodule-2.0
])

### GTK-DOC ###################################################################

# Allow gtkdocize to automatically copy the macro definition for
# GTK_DOC_CHECK to the project.
# AC_CONFIG_MACRO_DIR(m4)

### OUTPUT ####################################################################

# Output platform-specific definitions to config.h
AC_CONFIG_HEADERS([config.h])
# List of other files for Autoconf to output
AC_CONFIG_FILES([
Makefile
chimara.pc
chimara-plugin.pc
src/Makefile
interpreters/Makefile
interpreters/frotz/Makefile
interpreters/nitfol/Makefile
docs/Makefile
docs/reference/Makefile
docs/reference/version.xml
po/Makefile.in
])
# Do it
AC_OUTPUT

