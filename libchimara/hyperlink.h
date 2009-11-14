#ifndef HYPERLINK_H
#define HYPERLINK_H

#include <glib.h>
#include <gtk/gtk.h>

#include "glk.h"
#include "window.h"
#include "event.h"

G_GNUC_INTERNAL gboolean on_window_button_release_event(GtkWidget *widget, GdkEventButton *event, winid_t win);

#endif
