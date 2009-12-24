/*
 * Compile me with:
 * gcc -g -O0 -Wall -pthread -o barf barf.c `pkg-config --cflags --libs gtk+-2.0`
 */

#include <gtk/gtk.h>

static gulong pager_handler = 0;
static gulong expose_handler = 0;
static gboolean currently_paging = FALSE;

static gboolean
quit()
{
	gtk_main_quit();
	return TRUE;
}

static void
barf(GtkButton *button, GtkTextBuffer *buffer)
{
	GtkTextIter end;
	gtk_text_buffer_get_end_iter(buffer, &end);
	
	gtk_text_buffer_move_mark_by_name(buffer, "pager_position", &end);
	
	gchar *loremipsum;
	g_file_get_contents("loremipsum.txt", &loremipsum, NULL, NULL);
	gtk_text_buffer_insert(buffer, &end, loremipsum, -1);
	g_free(loremipsum);
}

static gint
move_pager_and_get_scroll_distance(GtkTextView *textview)
{
	GdkRectangle pagerpos, endpos, visiblerect;
	GtkTextIter pager, end;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(textview);
	
	gtk_text_view_get_visible_rect(textview, &visiblerect);
	gtk_text_view_get_iter_at_location(textview, &pager, visiblerect.x + visiblerect.width, visiblerect.y + visiblerect.height);
	gtk_text_buffer_move_mark_by_name(buffer, "pager_position", &pager);

	gtk_text_buffer_get_end_iter(buffer, &end);
	gtk_text_view_get_iter_location(textview, &pager, &pagerpos);
	gtk_text_view_get_iter_location(textview, &end, &endpos);
	
	g_printerr("View height = %d\n", visiblerect.height);
	g_printerr("End - Pager = %d\n", endpos.y - pagerpos.y);
	
	return endpos.y - pagerpos.y;
}

static void
start_paging(GtkTextView *textview)
{
	currently_paging = TRUE;
	g_signal_handler_unblock(textview, expose_handler);
	g_signal_handler_unblock(textview, pager_handler);
}

static void
stop_paging(GtkTextView *textview)
{
	currently_paging = FALSE;
	g_signal_handler_block(textview, expose_handler);
	g_signal_handler_block(textview, pager_handler);
}

static void
after_insert(GtkTextBuffer *buffer, GtkTextIter *location, gchar *text, gint len, GtkTextView *textview)
{
	while(gtk_events_pending())
		gtk_main_iteration();
	
	/* Move the pager to the last visible character in the buffer */
	gint scroll_distance = move_pager_and_get_scroll_distance(textview);
	
	/* Wait for a keypress to advance the pager */
	if(scroll_distance > 0 && !currently_paging)
		start_paging(textview);
}

static gboolean
pager_wait(GtkTextView *textview, GdkEventKey *event, GtkTextBuffer *buffer)
{
	GtkTextMark *pagermark = gtk_text_buffer_get_mark(buffer, "pager_position");
	gtk_text_view_scroll_to_mark(textview, pagermark, 0.0, TRUE, 0.0, 0.0);

	/*** ALTERNATIVE, POSSIBLY BETTER, METHOD OF SCROLLING ***
	GtkWidget *scrolledwindow = gtk_widget_get_parent(GTK_WIDGET(textview));
	GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolledwindow));
	gdouble page_size; // SUCKY_DEBIAN: use gtk_adjustment_get_page_size() post 2.12
	g_object_get(adjustment, "page-size", &page_size, NULL);
	gtk_adjustment_set_value(adjustment, gtk_adjustment_get_value(adjustment) + page_size);
	*/

	while(gtk_events_pending())
		gtk_main_iteration();

	/* Move the pager to the last visible character in the buffer */
	gint scroll_distance = move_pager_and_get_scroll_distance(textview);

	if(scroll_distance == 0)
		stop_paging(textview);

	return TRUE; /* block further handlers */
}

static gboolean
expose_prompt(GtkTextView *textview, GdkEventExpose *event)
{
	/* Use Cairo? Cairo supported on Iliad? */
	PangoLayout *prompt = gtk_widget_create_pango_layout(GTK_WIDGET(textview), "More");
	gint promptwidth, promptheight;
	pango_layout_get_pixel_size(prompt, &promptwidth, &promptheight);
	
	GdkGC *context = gdk_gc_new(GDK_DRAWABLE(event->window));
	GdkColor red, white;
	gdk_color_parse("red", &red);
	gdk_color_parse("white", &white);

	gint winx, winy, winwidth, winheight;
	gdk_window_get_position(event->window, &winx, &winy);
	gdk_drawable_get_size(GDK_DRAWABLE(event->window), &winwidth, &winheight);

	gdk_draw_layout_with_colors(event->window, context, 
		winx + winwidth - promptwidth, 
		winy + winheight - promptheight, 
		prompt, &white, &red);
	
	return FALSE; /* Propagate event further */
}

int
main(int argc, char **argv)
{
	gtk_init(&argc, &argv);
	
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request(window, 400, 400);
	GtkWidget *button = gtk_button_new_with_label("Barf");
	GtkWidget *scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
	GtkWidget *textview = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD_CHAR);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
	GtkWidget *vbox = gtk_vbox_new(FALSE, 6);
	
	gtk_container_add(GTK_CONTAINER(scrolledwindow), textview);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), scrolledwindow, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
	
	/* Set up the textview widget to receive exposure events, must be done after widget has been shown */
	gdk_window_set_events(gtk_widget_get_window(textview), GDK_EXPOSURE_MASK);

	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	g_signal_connect_after(buffer, "insert-text", G_CALLBACK(after_insert), textview);
	pager_handler = g_signal_connect(textview, "key-press-event", G_CALLBACK(pager_wait), buffer);
	g_signal_handler_block(textview, pager_handler);
	expose_handler = g_signal_connect_after(textview, "expose-event", G_CALLBACK(expose_prompt), NULL);
	g_signal_handler_block(textview, expose_handler);
	
	GtkTextIter end;
	gtk_text_buffer_get_end_iter(buffer, &end);
	GtkTextMark *pagermark = gtk_text_buffer_create_mark(buffer, "pager_position", &end, TRUE);
	gtk_text_mark_set_visible(pagermark, TRUE);
	
	g_signal_connect(window, "delete-event", G_CALLBACK(quit), NULL);
	g_signal_connect(button, "clicked", G_CALLBACK(barf), buffer);
	
	gtk_main();
	return 0;
}
