#ifndef UI_GRID_H
#define UI_GRID_H

#include <glib.h>

#include "chimara-glk.h"
#include "glk.h"

G_GNUC_INTERNAL void ui_grid_create(winid_t win, ChimaraGlk *glk);
G_GNUC_INTERNAL void ui_grid_clear(winid_t win);
G_GNUC_INTERNAL void ui_grid_move_cursor(winid_t win, unsigned xpos, unsigned ypos);
G_GNUC_INTERNAL void ui_grid_newline_cursor(winid_t win);
G_GNUC_INTERNAL gint64 ui_grid_cancel_line_input(winid_t win);

#endif /* UI_GRID_H */