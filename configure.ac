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
AC_PROG_INSTALL            # Install
LT_INIT                    # Libtool
AC_PROG_LIBTOOL            # Old way of declaring Libtool
AM_GLIB_GNU_GETTEXT        # Use GLib gettext instead of standard
IT_PROG_INTLTOOL([0.35.0]) # Intltool
PKG_PROG_PKG_CONFIG        # pkg_config
GTK_DOC_CHECK(1.9)         # Gtk-Doc

### INTERNATIONALIZATION ######################################################

GETTEXT_PACKAGE=chimara
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

### CHECK FOR LIBRARIES #######################################################

# Libraries needed to build Chimara library
PKG_CHECK_MODULES([CHIMARA], [
	glib-2.0 >= 2.6
	gtk+-2.0 >= 2.8 
	gthread-2.0 
	gmodule-2.0
])
AC_SUBST(CHIMARA_CFLAGS)
AC_SUBST(CHIMARA_LIBS)
# Libraries needed to build test programs
PKG_CHECK_MODULES([TEST], [
	gtk+-2.0 >= 2.8 
	gmodule-2.0
])
AC_SUBST(TEST_CFLAGS)
AC_SUBST(TEST_LIBS)

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
src/Makefile
docs/Makefile
docs/reference/Makefile
docs/reference/version.xml
po/Makefile.in
])
# Do it
AC_OUTPUT

