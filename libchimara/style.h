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
static gboolean style_accept(GScanner *scanner, GTokenType token);
static gboolean style_accept_style_identifier(GScanner *scanner);
static gboolean style_accept_style_hint(GScanner *scanner, GtkTextTag *current_tag);
static void style_add_tag_to_textbuffer(gpointer key, gpointer tag, gpointer tag_table);

#endif
