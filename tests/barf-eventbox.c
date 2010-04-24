/*
 * Compile me with:
 * gcc -g -O0 -Wall -pthread -o barf barf.c `pkg-config --cflags --libs gtk+-2.0`
 */

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

/* Uncomment to try the other paging method */
/* #define PAGE_ONLY_UNSEEN 1 */

static gulong pager_handler = 0;
static gulong expose_handler = 0;
static gboolean currently_paging = FALSE;

static gint promptwidth, promptheight;
static PangoLayout *prompt;
static GdkColor red, white;

static gboolean
quit()
{
	gtk_main_quit();
	return TRUE;
}

/* Vomit a load of text onto the screen */
static void
barf(GtkButton *button, GtkTextView *textview)
{
	GtkTextIter end;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(textview);
	gtk_text_buffer_get_end_iter(buffer, &end);
	
	gchar *loremipsum;
	g_file_get_contents("loremipsum.txt", &loremipsum, NULL, NULL);
	gtk_text_buffer_insert(buffer, &end, loremipsum, -1);
	g_free(loremipsum);
	
	gtk_widget_grab_focus(GTK_WIDGET(textview));
}

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
	
#ifdef PAGE_ONLY_UNSEEN
	if(gtk_text_iter_compare(&oldpager, &newpager) < 0)
		gtk_text_buffer_move_mark(buffer, pager, &newpager);
#else
	gtk_text_buffer_move_mark(buffer, pager, &newpager);
#endif

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
start_paging(GtkWidget *eventbox)
{
	currently_paging = TRUE;
	g_signal_handler_unblock(eventbox, expose_handler);
	g_signal_handler_unblock(eventbox, pager_handler);
}

/* Helper function: turn off paging for this textview */
static void
stop_paging(GtkWidget *eventbox)
{
	currently_paging = FALSE;
	g_signal_handler_block(eventbox, expose_handler);
	g_signal_handler_block(eventbox, pager_handler);
}

/* Update the pager position after new text is inserted in the buffer */
static void
after_insert(GtkTextBuffer *buffer, GtkTextIter *location, gchar *text, gint len, GtkTextView *textview)
{
	while(gtk_events_pending())
		gtk_main_iteration();
	
	/* Move the pager to the last visible character in the buffer */
	gint scroll_distance = move_pager_and_get_scroll_distance(textview);
	
	if(scroll_distance > 0 && !currently_paging) {
		GtkWidget *eventbox = gtk_widget_get_parent( gtk_widget_get_parent(GTK_WIDGET(textview)) );
		start_paging(eventbox);
	}
}

static void
adjustment_changed(GtkAdjustment *adjustment, GtkTextView *textview)
{
	while(gtk_events_pending())
		gtk_main_iteration();
	
	/* Move the pager, etc. */
	gint scroll_distance = move_pager_and_get_scroll_distance(textview);
	
	GtkWidget *eventbox = gtk_widget_get_parent( gtk_widget_get_parent(GTK_WIDGET(textview)) );
	if(scroll_distance > 0 && !currently_paging)
		start_paging(eventbox);
	else if(scroll_distance == 0 && currently_paging)
		stop_paging(eventbox);
	
	/* Refresh the widget so that any extra "more" prompts disappear */
	gtk_widget_queue_draw(GTK_WIDGET(eventbox));
}

/* Handle key press events in the textview while paging is active */
static gboolean
pager_wait(GtkTextView *eventbox, GdkEventKey *event, GtkTextBuffer *buffer)
{
	/*** ALTERNATIVE, POSSIBLY INFERIOR, METHOD OF SCROLLING ***
	GtkTextMark *pagermark = gtk_text_buffer_get_mark(buffer, "pager_position");
	gtk_text_view_scroll_to_mark(textview, pagermark, 0.0, TRUE, 0.0, 0.0);
	*/

	GtkWidget *scrolledwindow = gtk_bin_get_child(GTK_BIN(eventbox));
	GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolledwindow));
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
static gboolean
expose_prompt(GtkEventBox *eventbox, GdkEventExpose *event)
{
	g_printerr("Eventbox expose callback!\n");

	while(gtk_events_pending())
		gtk_main_iteration();
	
	GdkGC *context = gdk_gc_new(GDK_DRAWABLE(event->window));

	gint winx, winy, winwidth, winheight;
	gdk_window_get_position(event->window, &winx, &winy);
	gdk_drawable_get_size(GDK_DRAWABLE(event->window), &winwidth, &winheight);

	gdk_draw_layout_with_colors(event->window, context, 
		winx + winwidth - promptwidth, 
		//winy + winheight - promptheight, 
		winy - 5, 
		prompt, &white, &red);
	
	return FALSE; /* Propagate event further */
}

static gboolean
expose_scroll(GtkScrolledWindow *scroll, GdkEventExpose *event)
{
	g_printerr("Scrolled window expose callback!\n");
	return FALSE;
}

static gboolean
expose_textview(GtkTextView *textview, GdkEventExpose *event)
{
	g_printerr("Text view expose callback!\n");
	return FALSE;
}

int
main(int argc, char **argv)
{
	gtk_init(&argc, &argv);
	
	/* Construct the widgets */
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request(window, 400, 400);
	GtkWidget *button = gtk_button_new_with_label("Barf");
	GtkWidget *eventbox = gtk_event_box_new();
//	gtk_event_box_set_above_child( GTK_EVENT_BOX(eventbox), TRUE );
	GTK_WIDGET_SET_FLAGS(eventbox, GTK_APP_PAINTABLE);
	GtkWidget *scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
	GtkWidget *textview = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD_CHAR);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
	GtkWidget *vbox = gtk_vbox_new(FALSE, 6);
	
	/* Pack all the widgets into their containers */
	gtk_container_add(GTK_CONTAINER(scrolledwindow), textview);
	gtk_container_add(GTK_CONTAINER(eventbox), scrolledwindow);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), eventbox, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
	
	/* Set up the textview widget to receive exposure events, must be done after widget has been shown */
	gdk_window_set_events(gtk_widget_get_window(eventbox), GDK_EXPOSURE_MASK);

	/* Create the 'more' prompt */
	prompt = gtk_widget_create_pango_layout(GTK_WIDGET(textview), "More");
	pango_layout_get_pixel_size(prompt, &promptwidth, &promptheight);
	gdk_color_parse("red", &red);
	gdk_color_parse("white", &white);

	/* Connect paging signals */
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	g_signal_connect_after(buffer, "insert-text", G_CALLBACK(after_insert), textview);
	GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolledwindow));
	g_signal_connect_after(adj, "value-changed", G_CALLBACK(adjustment_changed), textview);
	pager_handler = g_signal_connect(eventbox, "key-press-event", G_CALLBACK(pager_wait), buffer);
	g_signal_handler_block(eventbox, pager_handler);
	expose_handler = g_signal_connect_after(eventbox, "expose-event", G_CALLBACK(expose_prompt), NULL);
	g_signal_handler_block(eventbox, expose_handler);

	g_signal_connect(scrolledwindow, "expose-event", G_CALLBACK(expose_scroll), NULL);
	g_signal_connect(textview, "expose-event", G_CALLBACK(expose_textview), NULL);
	
	/* Create the pager position mark; it stands for the last character in the buffer
	 that has been on-screen */
	GtkTextIter end;
	gtk_text_buffer_get_end_iter(buffer, &end);
	GtkTextMark *pagermark = gtk_text_buffer_create_mark(buffer, "pager_position", &end, TRUE);
	gtk_text_mark_set_visible(pagermark, TRUE);
	
	/* Connect "regular program" signals */
	g_signal_connect(window, "delete-event", G_CALLBACK(quit), NULL);
	g_signal_connect(button, "clicked", G_CALLBACK(barf), textview);
	
	gtk_main();

	g_object_unref(prompt);
	return 0;
}
