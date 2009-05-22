#ifndef INPUT_H
#define INPUT_H

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

#include "window.h"
#include "event.h"

G_GNUC_INTERNAL gboolean on_window_key_press_event(GtkWidget *widget, GdkEventKey *event, winid_t win);
G_GNUC_INTERNAL void after_window_insert_text(GtkTextBuffer *textbuffer, GtkTextIter *location, gchar *text, gint len, winid_t win);
G_GNUC_INTERNAL void on_input_entry_activate(GtkEntry *input_entry, winid_t win);
G_GNUC_INTERNAL static int flush_text_buffer(winid_t win);
G_GNUC_INTERNAL static int flush_text_grid(winid_t win);

#endif
