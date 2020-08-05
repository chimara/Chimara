#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

struct image_info {
	uint32_t resource_number;
	int width;
	int height;
	GdkPixbuf *pixbuf;
	gboolean scaled;
};

#endif
