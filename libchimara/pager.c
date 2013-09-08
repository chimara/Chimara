#include <gdk/gdkkeysyms.h>

#include "pager.h"

/* Not sure if necessary, but this is the margin within which the pager will
stop paging if it's close to the end of the text buffer */
#define PAGER_FUZZINESS 1.0

/* Helper function: move the pager to the last visible position in the buffer,
 and return the distance between the pager and the end of the buffer in buffer
 coordinates */
static void
move_pager_and_get_scroll_distance(GtkTextView *textview, gint *view_height, gint *scroll_distance, gboolean move )
{
	GdkRectangle pagerpos, endpos, visiblerect;
	GtkTextIter oldpager, newpager, end;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(textview);
	GtkTextMark *pager = gtk_text_buffer_get_mark(buffer, "pager_position");
	
	/* Get an iter at the lower right corner of the visible part of the buffer */
	gtk_text_view_get_visible_rect(textview, &visiblerect);
	gtk_text_view_get_iter_at_location(
		textview,
		&newpager,
		visiblerect.x + visiblerect.width,
		visiblerect.y + visiblerect.height
	);
	gtk_text_buffer_get_iter_at_mark(buffer, &oldpager, pager);
	
	if(move)
		gtk_text_buffer_move_mark(buffer, pager, &newpager);

	/* Get the buffer coordinates of the pager and the end iter */
	gtk_text_buffer_get_end_iter(buffer, &end);
	gtk_text_buffer_get_iter_at_mark(buffer, &newpager, pager);
	gtk_text_view_get_iter_location(textview, &newpager, &pagerpos);
	gtk_text_view_get_iter_location(textview, &end, &endpos);

	*view_height = visiblerect.height;
	*scroll_distance = endpos.y - pagerpos.y;
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

/* Helper function: If the adjustment is at its maximum value, stop paging */
static void
check_paging(GtkAdjustment *adj, winid_t win)
{
	double page_size, upper, value;
	g_object_get(adj,
		"page-size", &page_size,
		"upper", &upper,
		"value", &value,
		NULL);
	if(value + PAGER_FUZZINESS >= upper - page_size && win->currently_paging)
		stop_paging(win);
}

/* When the user scrolls up in a textbuffer, start paging. */
void
pager_after_adjustment_changed(GtkAdjustment *adj, winid_t win)
{
	/* Move the pager, etc. */
	gint scroll_distance, view_height;
   	move_pager_and_get_scroll_distance( GTK_TEXT_VIEW(win->widget), &view_height, &scroll_distance, TRUE );

	if(scroll_distance > 0 && !win->currently_paging)
	{
		start_paging(win);
		return;
	}
	else if(scroll_distance == 0 && win->currently_paging)
	{
		stop_paging(win);
		gtk_widget_queue_draw(win->widget);
		return;
	}

	/* Refresh the widget so that any extra "more" prompts disappear */
	gtk_widget_queue_draw(win->widget);

	check_paging(adj, win);
}

/* Handle key press events in the textview while paging is active */
gboolean
pager_on_key_press_event(GtkTextView *textview, GdkEventKey *event, winid_t win)
{
	GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment( GTK_SCROLLED_WINDOW(win->frame) );
	gdouble page_size, upper, lower, value;
	g_object_get(adj, 
		"page-size", &page_size,
		"upper", &upper,
		"lower", &lower,
		"value", &value,
		NULL);
	
	switch (event->keyval) {
		case GDK_KEY_space: case GDK_KEY_KP_Space:
		case GDK_KEY_Page_Down: case GDK_KEY_KP_Page_Down:
		case GDK_KEY_Return: case GDK_KEY_KP_Enter:
			gtk_adjustment_set_value(adj, CLAMP(value + page_size, lower, upper - page_size));
			check_paging(adj, win);
			return TRUE;
		case GDK_KEY_Page_Up: case GDK_KEY_KP_Page_Up:
			gtk_adjustment_set_value(adj, CLAMP(value - page_size, lower, upper - page_size));
			check_paging(adj, win);
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

	int winx, winy;
	gdk_window_get_position(event->window, &winx, &winy);
	int winwidth = gdk_window_get_width(event->window);
	int winheight = gdk_window_get_height(event->window);

	/* Draw the 'more' tag */
	cairo_t *cr = gdk_cairo_create(event->window);
	cairo_move_to(cr,
		winx + winwidth - promptwidth,
		winy + winheight - promptheight);
	pango_cairo_show_layout(cr, win->pager_layout);

	return FALSE; /* Propagate event further */
}

/* Check whether paging should be done. This function is called after the
 * textview has finished validating text positions. */
void 
pager_after_size_request(GtkTextView *textview, GtkRequisition *requisition, winid_t win) 
{
	/* Move the pager to the last visible character in the buffer */
	gint view_height, scroll_distance; 
	move_pager_and_get_scroll_distance(GTK_TEXT_VIEW(win->widget), &view_height, &scroll_distance, FALSE);

	if(view_height <= 1)
		/* Paging is unusable when window is too small */
		return;

	/* If not in interactive mode, then just scroll to the bottom. */
	ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(GTK_WIDGET(textview), CHIMARA_TYPE_GLK));
	g_assert(glk);
	if(!chimara_glk_get_interactive(glk)) {
		GtkTextIter end;
		gtk_text_buffer_get_end_iter(gtk_text_view_get_buffer(textview), &end);
		gtk_text_view_scroll_to_iter(textview, &end, 0.0, TRUE, 0.0, 0.0);
		return;
	}
	
	/* Scroll past text already read by user. This is automatic scrolling, so disable the pager_ajustment_handler
	 * first, that acts on the belief the scolling is performed by the user. */
	GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(win->frame));
	g_signal_handler_block(adj, win->pager_adjustment_handler);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(win->widget));
	GtkTextMark *pager_position = gtk_text_buffer_get_mark(buffer, "pager_position");
	gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(win->widget), pager_position, 0.0, TRUE, 0.0, 0.0);
	g_signal_handler_unblock(adj, win->pager_adjustment_handler);
	
	if(!win->currently_paging) {
		if(scroll_distance > view_height) {
			start_paging(win);
			gdk_window_invalidate_rect(gtk_widget_get_window(win->widget), NULL, TRUE);
		}
	}
}

void
pager_update(winid_t win)
{
	GtkTextIter input_iter;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(win->widget));
	GtkTextMark *input_position = gtk_text_buffer_get_mark(buffer, "input_position");
	GtkTextMark *pager_position = gtk_text_buffer_get_mark(buffer, "pager_position");
	gtk_text_buffer_get_iter_at_mark(buffer, &input_iter, input_position);
	gtk_text_buffer_move_mark(buffer, pager_position, &input_iter);
}
