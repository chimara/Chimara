#ifndef STYLE_H
#define STYLE_H

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include "glk.h"
#include "magic.h"
#include "chimara-glk-private.h"
#include "stream.h"

G_GNUC_INTERNAL void style_init_textbuffer(GtkTextBuffer *buffer);
G_GNUC_INTERNAL void style_init();

#endif
