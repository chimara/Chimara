#!/bin/sh
### autogen.sh with sensible comments ###############################

### GTK-DOC #########################################################
# Run before autotools
echo "Setting up Gtk-Doc"
gtkdocize || exit 1

### AUTOTOOLS #######################################################
# Runs autoconf, autoheader, aclocal, automake, autopoint, libtoolize
echo "Regenerating autotools files"
autoreconf --install --symlink || exit 1

### GLIB-GETTEXT ####################################################
echo "Running glib-gettextize... Ignore non-fatal messages"
glib-gettextize --force --copy || exit 1

### INTLTOOL ########################################################
# Run after autopoint or glib-gettextize
echo "Setting up Intltool"
intltoolize --copy --force --automake || exit 1
