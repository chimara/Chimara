#ifndef UI_BUFFER_H
#define UI_BUFFER_H

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "chimara-glk.h"
#include "glk.h"

G_GNUC_INTERNAL void ui_buffer_create(winid_t win, ChimaraGlk *glk);
G_GNUC_INTERNAL void ui_buffer_print_string(winid_t win, const char *text);
G_GNUC_INTERNAL void ui_buffer_clear(winid_t win);
G_GNUC_INTERNAL void ui_buffer_request_line_event(winid_t win, glui32 maxlen, gboolean insert, const char *inserttext);
G_GNUC_INTERNAL int ui_buffer_cancel_line_input(winid_t win);
G_GNUC_INTERNAL int ui_buffer_force_line_input(winid_t win, const char *text);
G_GNUC_INTERNAL void ui_buffer_draw_image(winid_t win, GdkPixbuf *pixbuf, glui32 alignment);

#endif /* UI_BUFFER_H */
