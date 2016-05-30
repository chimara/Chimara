#ifndef STYLE_H
#define STYLE_H

#include <gtk/gtk.h>

#include "chimara-glk.h"
#include "glk.h"

G_GNUC_INTERNAL void style_init_textbuffer(GtkTextBuffer *buffer);
G_GNUC_INTERNAL void style_init_textgrid(GtkTextBuffer *buffer);
G_GNUC_INTERNAL GScanner *create_css_file_scanner(void);
G_GNUC_INTERNAL void scan_css_file(GScanner *scanner, ChimaraGlk *glk);
G_GNUC_INTERNAL PangoFontDescription *get_current_font(guint32 wintype);
G_GNUC_INTERNAL GtkTextTag* gtk_text_tag_copy(GtkTextTag *tag);
G_GNUC_INTERNAL void glkcolor_to_gdkrgba(glui32 val, GdkRGBA *color);
G_GNUC_INTERNAL void style_stream_colors(strid_t str, GdkRGBA **foreground, GdkRGBA **background);
G_GNUC_INTERNAL void style_apply(winid_t win, GtkTextIter *start, GtkTextIter *end);

#endif
