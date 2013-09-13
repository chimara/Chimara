#include <gtk/gtk.h>
#include <libchimara/chimara-if.h>

void
on_command(ChimaraIF *glk, gchar *input, gchar *response, GtkWindow *window)
{
	GtkWidget *dialog = gtk_message_dialog_new(window, 
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
		"%s", input);
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", 
		response);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void
on_stop(GtkButton *button, ChimaraIF *glk)
{
	chimara_glk_stop(CHIMARA_GLK(glk));
	chimara_glk_wait(CHIMARA_GLK(glk));
}

void
on_go(GtkButton *button, ChimaraIF *glk)
{
	on_stop(button, glk);
	g_assert(chimara_if_run_game(CHIMARA_IF(glk), "unicodetest.ulx", NULL));
}

int
main(int argc, char *argv[])
{
    GtkWidget *window, *vbox, *hbox, *stop, *go, *glk;

    /* Initialize threads and GTK */
    gdk_threads_init();
    gtk_init(&argc, &argv);
    
    /* Construct the window and its contents. We quit the GTK main loop
     * when the window's close button is clicked. */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);
	vbox = gtk_grid_new();
	gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox), GTK_ORIENTATION_VERTICAL);
	gtk_grid_set_row_spacing(GTK_GRID(vbox), 6);
    glk = chimara_if_new();
	gtk_widget_set_hexpand(glk, TRUE);
	gtk_widget_set_vexpand(glk, TRUE);
    //g_signal_connect(glk, "command", G_CALLBACK(on_command), window);
	hbox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    stop = gtk_button_new_with_label("Stop");
    g_signal_connect(stop, "clicked", G_CALLBACK(on_stop), glk);
    go = gtk_button_new_with_label("Go");
    g_signal_connect(go, "clicked", G_CALLBACK(on_go), glk);

    gtk_box_pack_start(GTK_BOX(hbox), stop, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), go, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(vbox), hbox);
	gtk_container_add(GTK_CONTAINER(vbox), glk);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window);
    
    /* Add a reference to the ChimaraGlk widget, because we want to keep it
    around after gtk_main() exits */
    g_object_ref(glk);
    
    /* Start the plugin */
    g_assert(chimara_if_run_game(CHIMARA_IF(glk), "unicodetest.ulx", NULL));
    
    /* Start the GTK main loop */
    gdk_threads_enter();
    gtk_main();
    gdk_threads_leave();

    /* After the GTK main loop exits, signal the Glk program to shut down if
     * it is still running, and wait for it to exit. */
    chimara_glk_stop(CHIMARA_GLK(glk));
    chimara_glk_wait(CHIMARA_GLK(glk));
    g_object_unref(glk);

    return 0;
}
