#ifndef UI_WINDOW_H
#define UI_WINDOW_H

#include <gtk/gtk.h>
#include <pango/pango.h>

#include "chimara-glk.h"
#include "glk.h"

G_GNUC_INTERNAL void ui_window_arrange(ChimaraGlk *glk, gboolean suppress_next_arrange_event);
G_GNUC_INTERNAL void ui_window_create(winid_t win, ChimaraGlk *glk);
G_GNUC_INTERNAL void ui_window_clear(winid_t win);
G_GNUC_INTERNAL void ui_window_override_font(winid_t win, GtkWidget *widget, PangoFontDescription *font);
G_GNUC_INTERNAL void ui_window_override_background_color(winid_t win, GtkWidget *widget, GdkRGBA *color);
G_GNUC_INTERNAL gboolean ui_window_handle_shutdown_key_press(GtkWidget *widget, GdkEventKey *event, winid_t win);
G_GNUC_INTERNAL void ui_window_request_char_input(ChimaraGlk *glk, winid_t win, gboolean unicode);
G_GNUC_INTERNAL gboolean ui_window_handle_char_input_key_press(GtkWidget *widget, GdkEventKey *event, winid_t win);
G_GNUC_INTERNAL void ui_window_cancel_char_input(winid_t win);
G_GNUC_INTERNAL void ui_window_force_char_input(winid_t win, ChimaraGlk *glk, unsigned keyval);

#endif /* UI_WINDOW_H */
