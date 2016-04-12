#ifndef STYLE_H
#define STYLE_H

#include <gtk/gtk.h>

#include "chimara-glk.h"
#include "glk.h"

G_GNUC_INTERNAL void ui_style_init_text_buffer(ChimaraGlk *glk, GtkTextBuffer *buffer);
G_GNUC_INTERNAL void ui_style_init_text_grid(ChimaraGlk *glk, GtkTextBuffer *buffer);
G_GNUC_INTERNAL GScanner *create_css_file_scanner(void);
G_GNUC_INTERNAL void scan_css_file(GScanner *scanner, ChimaraGlk *glk);
G_GNUC_INTERNAL PangoFontDescription *ui_style_get_current_font(ChimaraGlk *glk, unsigned wintype);
G_GNUC_INTERNAL void glkcolor_to_gdkrgba(glui32 val, GdkRGBA *color);
G_GNUC_INTERNAL void ui_style_get_window_colors(winid_t win, GdkRGBA **foreground, GdkRGBA **background);
G_GNUC_INTERNAL void style_apply(winid_t win, GtkTextIter *start, GtkTextIter *end);

G_GNUC_INTERNAL void ui_style_apply_hint_to_tag(ChimaraGlk *glk, GtkTextTag *tag, unsigned wintype, unsigned styl, unsigned hint, int val);
G_GNUC_INTERNAL int ui_style_query_tag(ChimaraGlk *glk, GtkTextTag *tag, unsigned wintype, unsigned hint);
G_GNUC_INTERNAL const char *get_tag_name(unsigned style);
G_GNUC_INTERNAL const char *get_glk_tag_name(unsigned style);
G_GNUC_INTERNAL void ui_style_set_hint(ChimaraGlk *glk, unsigned wintype, unsigned styl, unsigned hint, int val);
G_GNUC_INTERNAL void ui_style_clear_hint(ChimaraGlk *glk, unsigned wintype, unsigned styl, unsigned hint);
G_GNUC_INTERNAL gint64 ui_window_measure_style(winid_t win, ChimaraGlk *glk, unsigned styl, unsigned hint);

#endif
