#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <gdk-pixbuf/gdk-pixbuf.h>

#define IMAGE_CACHE_MAX_NUM 10
#define IMAGE_CACHE_MAX_SIZE 5242880

struct image_info {
	guint32 resource_number;
	gint width, height;
	GdkPixbuf* pixbuf;
	gboolean scaled;
};

G_GNUC_INTERNAL void clear_image_cache(struct image_info *data, gpointer user_data);

#endif
