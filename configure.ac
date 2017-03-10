# configure.ac with sensible comments

### INITIALIZATION ############################################################

AC_PREREQ([2.64]) # Newest feature: m4_map_args_w
# Initialize Autoconf
# Args: (human-readable package name, package version, bug report address, 
# tarballname)
AC_INIT([chimara], [0.9.1])
# Sanity check to make sure we are running Autoconf from the right directory
AC_CONFIG_SRCDIR(libchimara/chimara-glk.c)
# Put m4 macros in their own directory
AC_CONFIG_MACRO_DIR([m4])
# Initialize Automake
AM_INIT_AUTOMAKE([-Wall -Wno-portability info-in-builddir])
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
GTK_REQUIRED_VERSION=3.10
GLIB_REQUIRED_VERSION=2.40
GTK_DOC_REQUIRED_VERSION=1.20
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
LT_INIT([dlopen])            # Libtool 2.2.6 and up
LT_PREREQ([2.2.6])
PKG_PROG_PKG_CONFIG          # pkg_config
GTK_DOC_CHECK($GTK_DOC_REQUIRED_VERSION) 
GLIB_GSETTINGS               # GSettings
# Other utilities used in this package's various make scripts
AC_PROG_AWK
AC_PATH_PROG([PERL], [perl]) # Perl
AC_PATH_PROG([TEST], [test]) # Test
AC_PATH_PROG([ECHO], [echo]) # Echo
AC_PATH_PROG([GLIB_COMPILE_RESOURCES], [glib-compile-resources])
AC_REQUIRE_AUX_FILE([tap-driver.sh])

### TYPES #####################################################################

AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT32_T

### INTERNATIONALIZATION ######################################################

AM_GNU_GETTEXT([external])  # Gettext, link to system libintl
AM_GNU_GETTEXT_VERSION([0.18.3])  # Supports Glade files
GETTEXT_PACKAGE=chimara
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

### INTROSPECTION ##############################################################

GOBJECT_INTROSPECTION_CHECK([0.6.7])

### RPM CONFIGURATION ##########################################################
# --enable-rpm requires rpm and rpmbuild
AC_PATH_PROG([RPMBUILD], [rpmbuild], [notfound])
AC_ARG_ENABLE([rpm],
	[AS_HELP_STRING([--enable-rpm=@<:@yes/no@:>@],
		[Configure for building RPM package @<:@default=no@:>@ (requires rpm and rpmbuild)])],
	[AS_IF([$TEST "x$enable_rpm" = xyes],
		[AS_IF([$TEST "x$RPMBUILD" = xnotfound],
			[AC_MSG_ERROR([rpmbuild is required for --enable-rpm])])])],
	[enable_rpm=no])
AM_CONDITIONAL(BUILDING_RPM, $TEST "x$enable_rpm" = xyes)

### SOUND LIBRARY TO USE ######################################################

AC_ARG_WITH([sound],
	[AS_HELP_STRING([--with-sound=@<:@no/auto/gstreamer0.10/gstreamer1.0@:>@],
		[Choose library with which to implement Glk sound])],
	[],
	[with_sound=auto])

have_gstreamer0_10=no
have_gstreamer1_0=no
AS_IF([test "x$with_sound" != "xno"], [
	PKG_CHECK_EXISTS([gstreamer-1.0], [have_gstreamer1_0=yes])
	PKG_CHECK_EXISTS([gstreamer-0.10], [have_gstreamer0_10=yes])])
SOUND_MODULE=
# Autodetect sound library
AS_IF([test "x$with_sound" = "xauto"], [
	AC_MSG_CHECKING([which sound library to use])
	AS_IF([test "x$have_gstreamer1_0" = "xyes"], [with_sound=gstreamer1.0], [
		AS_IF([test "x$have_gstreamer0_10" = "xyes"],
			[with_sound=gstreamer0.10],
			[with_sound=no])])
	AC_MSG_RESULT([$with_sound])])
# Sound library to use in the end
AS_CASE([$with_sound],
	[gstreamer0.10], [
		AC_DEFINE([GSTREAMER_0_10_SOUND], [1],
			[Define to enable sound support with GStreamer 0.10])
		SOUND_MODULE="gstreamer-0.10 >= 0.10.12"],
	[gstreamer1.0], [
		AC_DEFINE([GSTREAMER_1_0_SOUND], [1],
			[Define to enable sound support with GStreamer 1.0])
		SOUND_MODULE="gstreamer-1.0"],
	[])
AS_IF([test "x$with_sound" != "xno"],
	[AC_DEFINE([HAVE_SOUND], [1], [Define if any sound support is enabled])])

### WHETHER TO GENERATE A .VAPI FILE ##########################################
# Requires vapigen
AC_PATH_PROG([VAPIGEN], [vapigen], [notfound])
AM_CONDITIONAL(BUILDING_VAPI, $TEST "x$VAPIGEN" != xnotfound)

### WHETHER TO RUN TESTS IN VIRTUAL X SERVER ###################################
# Requires xvfb
AC_ARG_WITH([xvfb-tests],
	AS_HELP_STRING([--with-xvfb-tests],
		[Use xvfb to run tests, e.g. on build servers @<:@default=no@:>@]))
AS_IF([test "x$with_xvfb_tests" != "xno"],
	[AC_PATH_PROG([XVFB], [xvfb-run], [notfound])])
AS_IF([test "x$XVFB" != "xnotfound"],
	[AC_SUBST([XVFB_RUN], ["$XVFB "])],  dnl can add other xvfb-run args here
	[AS_IF([test "x$with_vfb_tests" = "yes"],
		[AC_MSG_ERROR([XVFB tests requested, but xvfb-run not found])])])


### CHECK FOR LIBRARIES #######################################################

# Libraries needed to build Chimara library
PKG_CHECK_MODULES([CHIMARA], [
	glib-2.0 >= $GLIB_REQUIRED_VERSION
	gtk+-3.0 >= $GTK_REQUIRED_VERSION
	gthread-2.0 
	gmodule-2.0
	pango
	$SOUND_MODULE
])
CHIMARA_LIBS="$CHIMARA_LIBS -lm"
AC_SUBST(CHIMARA_LIBS)
# Libraries needed to build Chimara player
PKG_CHECK_MODULES([PLAYER], [
	glib-2.0 >= $GLIB_REQUIRED_VERSION
	gtk+-3.0 >= $GTK_REQUIRED_VERSION
	gmodule-2.0
])
# Libraries needed to build test programs
PKG_CHECK_MODULES([TEST], [
	gtk+-3.0 >= $GTK_REQUIRED_VERSION
	gmodule-2.0 >= $GLIB_REQUIRED_VERSION
])

# GStreamer plugins needed to run library
m4_pattern_allow([AM_GST_ELEMENT_CHECK])
dnl In the case that gstreamer-0.10 isn't installed, AM_GST_ELEMENT_CHECK will
dnl be flagged as an undefined Automake macro
AS_CASE([$with_sound],
	[gstreamer0.10], [
		have_all_elements=yes
		m4_ifdef([AM_GST_ELEMENT_CHECK], [
			m4_map_args_w([
				giostreamsrc
				typefind
				audioconvert
				volume
				oggdemux
				vorbisdec
				autoaudiosink
				aiffparse
				modplug],
				[AM_GST_ELEMENT_CHECK(], [, [], [have_all_elements=no])])
			AS_IF([test "x$have_all_elements" = "xno"],
				[AC_MSG_ERROR([One or more required GStreamer elements were not found.
You will have to install your system's "base", "good", and "bad" plugins
pacakges. Try looking for names such as: gstreamer-plugins-base,
gstreamer-plugins-good, gstreamer-plugins-bad-free,
gstreamer-plugins-bad-free-extras, gstreamer0.10-plugins-base,
gstreamer0.10-plugins-good, gstreamer0.10-plugins-bad])])],
		[AC_MSG_ERROR([AM_GST_ELEMENT_CHECK not found. Install the development package
for GStreamer 0.10 and rerun autogen.sh.])])
	],
	[gstreamer1.0], [
		have_all_elements=yes
		m4_ifdef([GST_ELEMENT_CHECK], [
			m4_map_args_w([
				giostreamsrc
				typefind
				audioconvert
				volume
				oggdemux
				vorbisdec
				autoaudiosink
				aiffparse
				modplug],
				[GST_ELEMENT_CHECK(], [, [1.0], [], [have_all_elements=no])])
			AS_IF([test "x$have_all_elements" = "xno"],
				[AC_MSG_ERROR([One or more required GStreamer elements were not found.
You will have to install your system's "base", "good", and "bad" plugins
packages. Try looking for names such as: gstreamer1-plugins-base,
gstreamer1-plugins-good, gstreamer1-plugins-bad-free,
gstreamer1-plugins-bad-free-extras, gstreamer1.0-plugins-base,
gstreamer1.0-plugins-good, gstreamer1.0-plugins-bad])])],
			[AC_MSG_ERROR([GST_ELEMENT_CHECK not found. Install the development package
for GStreamer 1.0 and rerun autogen.sh.])])
	],
	[])

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
interpreters/bocfel/Makefile
tests/Makefile
tests/unit/Makefile
player/Makefile
player/config.py
docs/Makefile
docs/reference/Makefile
docs/reference/version.xml
docs/reference/build-selector-table.pl
po/Makefile.in
])

# Do it
AC_OUTPUT

