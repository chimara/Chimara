#ifndef HYPERLINK_H
#define HYPERLINK_H

#include <gtk/gtk.h>

#include "glk.h"

struct hyperlink {
	guint32 value;
	GtkTextTag *tag;
	gulong event_handler;
	winid_t window;
};
typedef struct hyperlink hyperlink_t;

G_GNUC_INTERNAL gboolean on_hyperlink_clicked(GtkTextTag *tag, GObject *object, GdkEvent *event, GtkTextIter *iter, hyperlink_t *link);
G_GNUC_INTERNAL void ui_window_set_hyperlink(winid_t win, unsigned linkval);
G_GNUC_INTERNAL void ui_window_request_hyperlink_input(winid_t win);
G_GNUC_INTERNAL void ui_window_cancel_hyperlink_input(winid_t win);

#endif
