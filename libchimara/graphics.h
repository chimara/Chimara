#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <glib.h>
#include <gtk/gtk.h>

#include "glk.h"
#include "gi_blorb.h"
#include "resource.h"
#include "window.h"

struct image_info {
	guint32 resource_number;
	gint width, height;
};

void on_graphics_size_allocate(GtkWidget *widget, GtkAllocation *allocation, winid_t win);

#endif
