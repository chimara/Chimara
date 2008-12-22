dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(chimara, 0.1)

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC

dnl ***************************************************************************
dnl Internatinalization
dnl ***************************************************************************
GETTEXT_PACKAGE=chimara
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])
AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL([0.35.0])

dnl AM_PROG_LIBTOOL

PKG_CHECK_MODULES([CHIMARA], [gtk+-2.0 >= 2.8 gthread-2.0])
AC_SUBST(CHIMARA_CFLAGS)
AC_SUBST(CHIMARA_LIBS)

##################################################
# Check for gtk-doc.
##################################################
AC_ARG_WITH(html-dir, [  --with-html-dir=PATH path to installed docs ])
if test "x$with_html_dir" = "x" ; then
  HTML_DIR='${datadir}/gtk-doc/html'
else
  HTML_DIR=$with_html_dir
fi
AC_SUBST(HTML_DIR)

gtk_doc_min_version=1.0
AC_MSG_CHECKING([gtk-doc version >= $gtk_doc_min_version])
if pkg-config --atleast-version=$gtk_doc_min_version gtk-doc; then
  AC_MSG_RESULT(yes)
  GTKDOC=true
else
  AC_MSG_RESULT(no)
  GTKDOC=false
fi
dnl Let people disable the gtk-doc stuff.
AC_ARG_ENABLE(gtk-doc,
              [  --enable-gtk-doc  Use gtk-doc to build documentation [default=auto]],
	      enable_gtk_doc="$enableval", enable_gtk_doc=auto)
if test x$enable_gtk_doc = xauto ; then
  if test x$GTKDOC = xtrue ; then
    enable_gtk_doc=yes
  else
    enable_gtk_doc=no
  fi
fi
AM_CONDITIONAL(ENABLE_GTK_DOC, test x$enable_gtk_doc = xyes)

AC_OUTPUT([
Makefile
src/Makefile
po/Makefile.in
])
