#include <gdk/gdkkeysyms.h>

#include "pager.h"

/* Helper function: move the pager to the last visible position in the buffer,
 and return the distance between the pager and the end of the buffer in buffer
 coordinates */
static gint
move_pager_and_get_scroll_distance(GtkTextView *textview)
{
	GdkRectangle pagerpos, endpos, visiblerect;
	GtkTextIter oldpager, newpager, end;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(textview);
	GtkTextMark *pager = gtk_text_buffer_get_mark(buffer, "pager_position");
	
	/* Get an iter at the lower right corner of the visible part of the buffer */
	gtk_text_view_get_visible_rect(textview, &visiblerect);
	gtk_text_view_get_iter_at_location(textview, &newpager, visiblerect.x + visiblerect.width, visiblerect.y + visiblerect.height);
	gtk_text_buffer_get_iter_at_mark(buffer, &oldpager, pager);
	
	gtk_text_buffer_move_mark(buffer, pager, &newpager);

	/* Get the buffer coordinates of the pager and the end iter */
	gtk_text_buffer_get_end_iter(buffer, &end);
	gtk_text_buffer_get_iter_at_mark(buffer, &newpager, pager);
	gtk_text_view_get_iter_location(textview, &newpager, &pagerpos);
	gtk_text_view_get_iter_location(textview, &end, &endpos);
	
	//g_printerr("View height = %d\n", visiblerect.height);
	//g_printerr("End - Pager = %d\n", endpos.y - pagerpos.y);
	
	return endpos.y - pagerpos.y;
}

/* Helper function: turn on paging for this textview */
static void
start_paging(winid_t win)
{
	win->currently_paging = TRUE;
	g_signal_handler_unblock(win->widget, win->pager_expose_handler);
	g_signal_handler_unblock(win->widget, win->pager_keypress_handler);
}

/* Helper function: turn off paging for this textview */
static void
stop_paging(winid_t win)
{
	win->currently_paging = FALSE;
	g_signal_handler_block(win->widget, win->pager_expose_handler);
	g_signal_handler_block(win->widget, win->pager_keypress_handler);
}

/* Update the pager position after new text is inserted in the buffer */
void
pager_after_insert_text(GtkTextBuffer *buffer, GtkTextIter *location, gchar *text, gint len, winid_t win)
{
	while(gtk_events_pending())
		gtk_main_iteration();
	
	/* Move the pager to the last visible character in the buffer */
	gint scroll_distance = move_pager_and_get_scroll_distance( GTK_TEXT_VIEW(win->widget) );
	
	if(scroll_distance > 0 && !win->currently_paging)
		start_paging(win);
}

void
pager_after_adjustment_changed(GtkAdjustment *adj, winid_t win)
{
	while(gtk_events_pending())
		gtk_main_iteration();
	
	/* Move the pager, etc. */
	gint scroll_distance = move_pager_and_get_scroll_distance( GTK_TEXT_VIEW(win->widget) );
	
	if(scroll_distance > 0 && !win->currently_paging)
		start_paging(win);
	else if(scroll_distance == 0 && win->currently_paging)
		stop_paging(win);
	
	/* Refresh the widget so that any extra "more" prompts disappear */
	gtk_widget_queue_draw(win->widget);
}

/* Handle key press events in the textview while paging is active */
gboolean
pager_on_key_press_event(GtkTextView *textview, GdkEventKey *event, winid_t win)
{
	/*** ALTERNATIVE, POSSIBLY INFERIOR, METHOD OF SCROLLING ***
	GtkTextMark *pagermark = gtk_text_buffer_get_mark(buffer, "pager_position");
	gtk_text_view_scroll_to_mark(textview, pagermark, 0.0, TRUE, 0.0, 0.0);
	*/

	GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment( GTK_SCROLLED_WINDOW(win->frame) );
	gdouble step_increment, page_size, upper, lower, value;
	g_object_get(adj, 
		"page-size", &page_size,
		"step-increment", &step_increment,
		"upper", &upper,
		"lower", &lower,
		"value", &value,
		NULL);
	
	switch (event->keyval) {
		case GDK_space: case GDK_KP_Space: case GDK_Page_Down: case GDK_KP_Page_Down:
			gtk_adjustment_set_value(adj, CLAMP(value + page_size, lower, upper - page_size));
			return TRUE;
		case GDK_Page_Up: case GDK_KP_Page_Up:
			gtk_adjustment_set_value(adj, CLAMP(value - page_size, lower, upper - page_size));
			return TRUE;
		case GDK_Return: case GDK_KP_Enter:
			gtk_adjustment_set_value(adj, CLAMP(value + step_increment, lower, upper - page_size));
			return TRUE;
			/* don't handle "up" and "down", they're used for input history */
	}
	
	return FALSE; /* if the key wasn't handled here, pass it to other handlers */
}

/* Draw the "more" prompt on top of the buffer, after the regular expose event has run */
gboolean
pager_on_expose(GtkTextView *textview, GdkEventExpose *event, winid_t win)
{
	/* Calculate the position of the 'more' tag */
	gint promptwidth, promptheight;
	pango_layout_get_pixel_size(win->pager_layout, &promptwidth, &promptheight);

	gint winx, winy, winwidth, winheight;
	gdk_window_get_position(event->window, &winx, &winy);
	gdk_drawable_get_size(GDK_DRAWABLE(event->window), &winwidth, &winheight);

	/* Draw the 'more' tag */
	GdkGC *context = gdk_gc_new(GDK_DRAWABLE(event->window));
	/*
	gdk_draw_layout_with_colors(event->window, context, 
		winx + winwidth - promptwidth, 
		winy + winheight - promptheight, 
		prompt, &white, &red);
	*/
	gdk_draw_layout(event->window, context, 
		winx + winwidth - promptwidth, 
		winy + winheight - promptheight, 
		win->pager_layout);

	return FALSE; /* Propagate event further */
}
