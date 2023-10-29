#include <gtk/gtk.h>
#include <libchimara/chimara-glk.h>

int
main(int argc, char *argv[])
{
	GError *error = NULL;

	gtk_init(&argc, &argv);

	GtkWidget *glk = chimara_glk_new();
	g_signal_connect(glk, "stopped", gtk_main_quit, NULL);

	GtkWidget *win = gtk_offscreen_window_new();
	gtk_container_add(GTK_CONTAINER(win), glk);
	gtk_widget_show_all(win);

	if(argc < 2)
		g_error("Must provide a plugin\n");

	g_autoptr(GFile) plugin_file = g_file_new_for_commandline_arg(argv[1]);

    if( !chimara_glk_run_file(CHIMARA_GLK(glk), plugin_file,
        argc - 1, argv + 1, &error) )
		g_error("Error starting Glk library: %s\n", error->message);

	gtk_main();

	chimara_glk_stop(CHIMARA_GLK(glk));
	chimara_glk_wait(CHIMARA_GLK(glk));

	gtk_widget_destroy(win);

	return 0;
}
