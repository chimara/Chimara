/*
 * Compile me with:
 * gcc -g -O0 -Wall -pthread -o barf barf.c `pkg-config --cflags --libs gtk+-2.0`
 */

#include <gtk/gtk.h>

static glong pager_handler = 0;

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

static void
after_insert(GtkTextBuffer *buffer, GtkTextIter *location, gchar *text, gint len, GtkTextView *textview)
{
	while(gtk_events_pending())
		gtk_main_iteration();

	GdkRectangle pagerpos, endpos, visiblerect;
	GtkTextIter pager, end;
	gtk_text_buffer_get_end_iter(buffer, &end);
	gtk_text_view_get_visible_rect(textview, &visiblerect);
	
	/* Move the pager to the last visible character in the buffer */
	gtk_text_view_get_line_at_y(textview, &pager, visiblerect.y, NULL);
	if(!gtk_text_iter_ends_line(&pager))
		gtk_text_iter_forward_to_line_end(&pager);
	gtk_text_buffer_move_mark_by_name(buffer, "pager_position", &pager);

	gtk_text_view_get_iter_location(textview, &pager, &pagerpos);
	gtk_text_view_get_iter_location(textview, &end, &endpos);
	
	g_printerr("View height = %d\n", visiblerect.height);
	g_printerr("End - Pager = %d\n", endpos.y - pagerpos.y);
	
	/* Wait for a keypress to advance the pager */
	if(endpos.y > pagerpos.y)
		g_signal_handler_unblock(textview, pager_handler);
}

static gboolean
pager_wait(GtkTextView *textview, GdkEventKey *event, GtkTextBuffer *buffer)
{
	GdkRectangle pagerpos, endpos, visiblerect;
	GtkTextIter pager, end;
	GtkTextMark *pagermark = gtk_text_buffer_get_mark(buffer, "pager_position");
	gtk_text_view_scroll_to_mark(textview, pagermark, 0.0, TRUE, 0.0, 0.0);

	while(gtk_events_pending())
		gtk_main_iteration();

	/* Move the pager to the last visible character in the buffer */
	gtk_text_view_get_line_at_y(textview, &pager, visiblerect.y, NULL);
	if(!gtk_text_iter_ends_line(&pager))
		gtk_text_iter_forward_to_line_end(&pager);
	gtk_text_buffer_move_mark(buffer, pagermark, &pager);

	gtk_text_buffer_get_end_iter(buffer, &end);
	gtk_text_view_get_visible_rect(textview, &visiblerect);
	gtk_text_view_get_iter_location(textview, &pager, &pagerpos);
	gtk_text_view_get_iter_location(textview, &end, &endpos);
	
	g_printerr("View height = %d\n", visiblerect.height);
	g_printerr("End - Pager = %d\n", endpos.y - pagerpos.y);

	if(endpos.y == pagerpos.y)
		g_signal_handler_block(textview, pager_handler);

	return TRUE; /* block further handlers */
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

	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	g_signal_connect_after(buffer, "insert-text", G_CALLBACK(after_insert), textview);
	pager_handler = g_signal_connect(textview, "key-press-event", G_CALLBACK(pager_wait), buffer);
	g_signal_handler_block(textview, pager_handler);
	
	GtkTextIter end;
	gtk_text_buffer_get_end_iter(buffer, &end);
	GtkTextMark *pagermark = gtk_text_buffer_create_mark(buffer, "pager_position", &end, TRUE);
	gtk_text_mark_set_visible(pagermark, TRUE);
	
	g_signal_connect(window, "delete-event", G_CALLBACK(quit), NULL);
	g_signal_connect(button, "clicked", G_CALLBACK(barf), buffer);
	
	gtk_main();
	return 0;
}
