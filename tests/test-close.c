#include <gtk/gtk.h>
#include <libchimara/chimara-if.h>

int
main(int argc, char *argv[])
{
    GtkWidget *window, *glk;

    /* Initialize threads and GTK */
    if(!g_thread_supported())
        g_thread_init(NULL);
    gdk_threads_init();
    gtk_init(&argc, &argv);
    
    /* Construct the window and its contents. We quit the GTK main loop
     * when the window's close button is clicked. */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);
    glk = chimara_glk_new();
    gtk_container_add(GTK_CONTAINER(window), glk);
    gtk_widget_show_all(window);
    
    /* Add a reference to the ChimaraGlk widget, because we want to keep it
    around after gtk_main() exits */
    g_object_ref(glk);
    
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
