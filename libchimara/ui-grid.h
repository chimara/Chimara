#ifndef UI_GRID_H
#define UI_GRID_H

#include <glib.h>

#include "chimara-glk.h"
#include "glk.h"

G_GNUC_INTERNAL void ui_grid_create(winid_t win, ChimaraGlk *glk);
G_GNUC_INTERNAL void ui_grid_print_string(winid_t win, const char *text);
G_GNUC_INTERNAL void ui_grid_clear(winid_t win);
G_GNUC_INTERNAL void ui_grid_move_cursor(winid_t win, unsigned xpos, unsigned ypos);
G_GNUC_INTERNAL void ui_grid_newline_cursor(winid_t win);
G_GNUC_INTERNAL void ui_grid_request_line_event(winid_t win, unsigned maxlen, gboolean insert, const char *inserttext);
G_GNUC_INTERNAL gint64 ui_grid_cancel_line_input(winid_t win);
G_GNUC_INTERNAL int ui_grid_force_line_input(winid_t win, const char *text);

#endif /* UI_GRID_H */