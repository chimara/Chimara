#ifndef STYLE_H
#define STYLE_H

#include <gtk/gtk.h>
#include <glib.h>

G_GNUC_INTERNAL void style_init_textbuffer(GtkTextBuffer *buffer);
G_GNUC_INTERNAL void style_init_textgrid(GtkTextBuffer *buffer);
G_GNUC_INTERNAL void style_init_more_prompt(winid_t win);
G_GNUC_INTERNAL void style_init();
G_GNUC_INTERNAL PangoFontDescription* get_current_font(guint32 wintype);
G_GNUC_INTERNAL GtkTextTag* gtk_text_tag_copy(GtkTextTag *tag);

G_GNUC_INTERNAL void glkcolor_to_gdkcolor(glui32 val, GdkColor *color);

typedef struct StyleSet {
	GHashTable *text_grid;
	GHashTable *text_buffer;
} StyleSet;

#endif
