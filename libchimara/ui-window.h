#ifndef UI_WINDOW_H
#define UI_WINDOW_H

#include <glib.h>

#include "chimara-glk.h"
#include "glk.h"

G_GNUC_INTERNAL void calculate_zero_character_size(GtkWidget *widget, PangoFontDescription *font, int *width, int *height);
G_GNUC_INTERNAL void ui_window_arrange(ChimaraGlk *glk, gboolean suppress_next_arrange_event);
G_GNUC_INTERNAL void ui_window_create(winid_t win, ChimaraGlk *glk);
G_GNUC_INTERNAL void ui_window_cancel_char_input(winid_t win);
G_GNUC_INTERNAL void ui_window_force_char_input(winid_t win, ChimaraGlk *glk, unsigned keyval);
G_GNUC_INTERNAL void ui_window_set_style(winid_t win, unsigned styl);

#endif /* UI_WINDOW_H */
