#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <gtk/gtk.h>

#include "glk.h"

#define IMAGE_CACHE_MAX_NUM 10
#define IMAGE_CACHE_MAX_SIZE 5242880

struct image_info {
	guint32 resource_number;
	gint width, height;
	GdkPixbuf* pixbuf;
	gboolean scaled;
};

gboolean on_graphics_configure(GtkWidget *widget, GdkEventConfigure *event, winid_t win);
gboolean on_graphics_draw(GtkWidget *widget, cairo_t *cr, winid_t win);
void clear_image_cache(struct image_info *data, gpointer user_data);

G_GNUC_INTERNAL void glkcairo_set_source_glkcolor(cairo_t *cr, glui32 val);
G_GNUC_INTERNAL void ui_graphics_fill_rect(winid_t win, glui32 color, glsi32 left, glsi32 top, glui32 width, glui32 height);

#endif
