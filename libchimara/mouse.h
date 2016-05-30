#ifndef MOUSE_H
#define MOUSE_H

#include <gtk/gtk.h>

#include "glk.h"

G_GNUC_INTERNAL gboolean on_window_button_press(GtkWidget *widget, GdkEventButton *event, winid_t win);

#endif
