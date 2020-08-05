#include <stdint.h>

#include <gtk/gtk.h>

#include "chimara-glk-private.h"
#include "event.h"
#include "glk.h"
#include "ui-window.h"
#include "window.h"

/* Signal handler for mouse clicks in graphics windows */
static gboolean
on_graphics_button_press(GtkWidget *widget, GdkEventButton *event, winid_t win)
{
	ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(win->widget, CHIMARA_TYPE_GLK));
	g_assert(glk);

	chimara_glk_push_event(glk, evtype_MouseInput, win, event->x, event->y);
	g_signal_handler_block(win->widget, win->button_press_event_handler);

	return GDK_EVENT_STOP;
}

/* Set the cairo context @cr's source color to be the Glk color @val */
static void
glkcairo_set_source_glkcolor(cairo_t *cr, uint32_t val)
{
	double r, g, b;
	r = ((val & 0xff0000) >> 16) / 256.0;
	g = ((val & 0x00ff00) >> 8) / 256.0;
	b = (val & 0x0000ff) / 256.0;
	cairo_set_source_rgb(cr, r, g, b);
}

/* Called when the graphics window is resized, restacked, or moved. Resize the
backing store if necessary. */
static gboolean
on_graphics_configure(GtkWidget *widget, GdkEventConfigure *event, winid_t win)
{
	int oldwidth = 0, oldheight = 0;

	/* Determine whether the backing store can stay the same size */
	gboolean needs_resize = FALSE;
	if(win->backing_store == NULL)
		needs_resize = TRUE;
	else {
		oldwidth = cairo_image_surface_get_width(win->backing_store);
		oldheight = cairo_image_surface_get_height(win->backing_store);
		if(oldwidth != event->width || oldheight != event->height)
			needs_resize = TRUE;
	}

	if(needs_resize) {
		/* Create a new backing store */
		cairo_surface_t *new_backing_store = gdk_window_create_similar_surface( gtk_widget_get_window(widget), CAIRO_CONTENT_COLOR, gtk_widget_get_allocated_width(widget), gtk_widget_get_allocated_height(widget) );
		cairo_t *cr = cairo_create(new_backing_store);

		/* Clear to background color */
		glkcairo_set_source_glkcolor(cr, win->background_color);
		cairo_paint(cr);

		if(win->backing_store != NULL) {
			/* Copy the contents of the old backing store */
			cairo_set_source_surface(cr, win->backing_store, 0, 0);
			cairo_paint(cr);
			cairo_surface_destroy(win->backing_store);
		}

		cairo_destroy(cr);
		/* Use the new backing store */
		win->backing_store = new_backing_store;
	}

	return GDK_EVENT_STOP;
}

/* Draw the backing store to the screen. Called whenever the drawing area is
exposed. */
static gboolean
on_graphics_draw(GtkWidget *widget, cairo_t *cr, winid_t win)
{
	cairo_set_source_surface(cr, win->backing_store, 0, 0);
	cairo_paint(cr);
	return GDK_EVENT_PROPAGATE;
}

/* Creates a graphics window, filling in the fields of @win.
 * Called as a result of glk_window_open(). */
void
ui_graphics_create(winid_t win)
{
	win->widget = win->frame = gtk_drawing_area_new();
	gtk_widget_show(win->widget);

	/* Connect signal handlers */
	win->button_press_event_handler = g_signal_connect(win->widget, "button-press-event", G_CALLBACK(on_graphics_button_press), win);
	g_signal_handler_block(win->widget, win->button_press_event_handler);
	win->shutdown_keypress_handler = g_signal_connect(win->widget, "key-press-event", G_CALLBACK(ui_window_handle_shutdown_key_press), win);
	g_signal_handler_block(win->widget, win->shutdown_keypress_handler);
	g_signal_connect(win->widget, "configure-event", G_CALLBACK(on_graphics_configure), win);
	g_signal_connect(win->widget, "draw", G_CALLBACK(on_graphics_draw), win);
}

/* Draws a rectangle of color @color at coordinates @x, @y, with the given
 * @width and @height, on the graphics window @win. */
void
ui_graphics_fill_rect(winid_t win, unsigned color, int left, int top, unsigned width, unsigned height)
{
	cairo_t *cr = cairo_create(win->backing_store);
	glkcairo_set_source_glkcolor(cr, color);
	cairo_rectangle(cr, (double)left, (double)top, (double)width, (double)height);
	cairo_fill(cr);
	gtk_widget_queue_draw(win->widget);
	cairo_destroy(cr);
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
