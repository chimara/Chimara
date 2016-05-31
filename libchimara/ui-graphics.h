#ifndef UI_GRAPHICS_H
#define UI_GRAPHICS_H

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "glk.h"

G_GNUC_INTERNAL void ui_graphics_create(winid_t win);
G_GNUC_INTERNAL void ui_graphics_fill_rect(winid_t win, unsigned color, int left, int top, unsigned width, unsigned height);
G_GNUC_INTERNAL void ui_graphics_clear(winid_t win);
G_GNUC_INTERNAL void ui_graphics_draw_image(winid_t win, GdkPixbuf *pixbuf, unsigned x, unsigned y);

#endif /* UI_GRAPHICS_H */
