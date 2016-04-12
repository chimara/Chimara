#ifndef INPUT_H
#define INPUT_H

#include <gtk/gtk.h>

#include "chimara-glk.h"
#include "glk.h"

G_GNUC_INTERNAL gboolean on_shutdown_key_press_event(GtkWidget *widget, GdkEventKey *event, winid_t win);
G_GNUC_INTERNAL gboolean on_char_input_key_press_event(GtkWidget *widget, GdkEventKey *event, winid_t win);
G_GNUC_INTERNAL gboolean on_line_input_key_press_event(GtkWidget *widget, GdkEventKey *event, winid_t win);
G_GNUC_INTERNAL void after_window_insert_text(GtkTextBuffer *textbuffer, GtkTextIter *location, gchar *text, gint len, winid_t win);
G_GNUC_INTERNAL void on_input_entry_activate(GtkEntry *input_entry, winid_t win);
G_GNUC_INTERNAL gboolean on_input_entry_key_press_event(GtkEntry *input_entry, GdkEventKey *event, winid_t win);
G_GNUC_INTERNAL void on_input_entry_changed(GtkEditable *editable, winid_t win);
G_GNUC_INTERNAL glui32 keyval_to_glk_keycode(guint keyval, gboolean unicode);
G_GNUC_INTERNAL void force_char_input_from_queue(winid_t win, event_t *event);
G_GNUC_INTERNAL void force_line_input_from_queue(winid_t win, event_t *event);
G_GNUC_INTERNAL gboolean is_valid_line_terminator(glui32 keycode);

G_GNUC_INTERNAL void ui_window_request_char_input(ChimaraGlk *glk, winid_t win, gboolean unicode);
G_GNUC_INTERNAL int ui_buffer_finish_line_input(winid_t win, gboolean emit_signal);
G_GNUC_INTERNAL int ui_grid_finish_line_input(winid_t win, gboolean emit_signal);
G_GNUC_INTERNAL void ui_window_request_line_input(ChimaraGlk *glk, winid_t win, glui32 maxlen, gboolean insert, const char *inserttext);
G_GNUC_INTERNAL int ui_window_force_line_input(winid_t win, const char *text);

#endif
