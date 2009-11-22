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
#include "strio.h"

G_GNUC_INTERNAL void style_init_textbuffer(GtkTextBuffer *buffer);
G_GNUC_INTERNAL void style_init_textgrid(GtkTextBuffer *buffer);
G_GNUC_INTERNAL void style_init();
G_GNUC_INTERNAL PangoFontDescription* get_current_font(guint32 wintype);
G_GNUC_INTERNAL GtkTextTag* gtk_text_tag_copy(GtkTextTag *tag);

typedef struct StyleSet {
	GHashTable *text_grid;
	GHashTable *text_buffer;
} StyleSet;

#endif
