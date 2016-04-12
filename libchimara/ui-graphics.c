#include <gtk/gtk.h>

#include "glk.h"
#include "graphics.h"
#include "input.h"
#include "mouse.h"
#include "window.h"

/* Creates a graphics window, filling in the fields of @win.
 * Called as a result of glk_window_open(). */
void
ui_graphics_create(winid_t win)
{
	win->widget = win->frame = gtk_drawing_area_new();
	gtk_widget_show(win->widget);

	/* Connect signal handlers */
	win->button_press_event_handler = g_signal_connect(win->widget, "button-press-event", G_CALLBACK(on_window_button_press), win);
	g_signal_handler_block(win->widget, win->button_press_event_handler);
	win->shutdown_keypress_handler = g_signal_connect(win->widget, "key-press-event", G_CALLBACK(on_shutdown_key_press_event), win);
	g_signal_handler_block(win->widget, win->shutdown_keypress_handler);
	g_signal_connect(win->widget, "configure-event", G_CALLBACK(on_graphics_configure), win);
	g_signal_connect(win->widget, "draw", G_CALLBACK(on_graphics_draw), win);
}

/* Clears the graphics window @win.
 * Called as a result of glk_window_clear(). */
void
ui_graphics_clear(winid_t win)
{
	GtkAllocation allocation;
	gtk_widget_get_allocation(win->widget, &allocation);
	ui_graphics_fill_rect(win, win->background_color, 0, 0, allocation.width, allocation.height);
}

/* Draws an image, stored in @pixbuf, at coordinates @x, @y, on the graphics
 * window @win.
 * Called as a result of glk_image_draw(). */
void
ui_graphics_draw_image(winid_t win, GdkPixbuf *pixbuf, unsigned x, unsigned y)
{
	cairo_t *cr = cairo_create(win->backing_store);
	gdk_cairo_set_source_pixbuf(cr, pixbuf, x, y);
	cairo_paint(cr);
	cairo_destroy(cr);

	/* Update the screen */
	gtk_widget_queue_draw(win->widget);
}
