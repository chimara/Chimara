#ifndef UI_STYLE_H
#define UI_STYLE_H

#include <stdint.h>

#include <gtk/gtk.h>

#include "chimara-glk.h"
#include "glk.h"

G_GNUC_INTERNAL void ui_style_set_hint(ChimaraGlk *glk, unsigned wintype, unsigned styl, unsigned hint, int val);
G_GNUC_INTERNAL void ui_style_clear_hint(ChimaraGlk *glk, unsigned wintype, unsigned styl, unsigned hint);
G_GNUC_INTERNAL int64_t ui_window_measure_style(winid_t win, ChimaraGlk *glk, unsigned styl, unsigned hint);
G_GNUC_INTERNAL PangoFontDescription *ui_style_get_current_font(ChimaraGlk *glk, unsigned wintype);
G_GNUC_INTERNAL void ui_style_get_window_colors(winid_t win, GdkRGBA **foreground, GdkRGBA **background);
G_GNUC_INTERNAL void ui_style_apply(winid_t win, GtkTextIter *start, GtkTextIter *end);

#endif /* UI_STYLE_H */
