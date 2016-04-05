#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include <libchimara/chimara-glk.h>

#include "plugin-utils.h"

int
main(int argc, char *argv[])
{
	GError *error = NULL;
	GtkWidget *glk = NULL;

	gdk_threads_init();
	gtk_init(&argc, &argv);

	glk = chimara_glk_new();
	g_object_ref(glk);
	g_signal_connect(glk, "stopped", gtk_main_quit, NULL);
	gtk_widget_show_all(glk);

	if(argc < 2)
		g_error("Must provide a plugin\n");

    GFile *plugin_file;
    if( g_str_has_suffix(argv[1], ".la") )
        plugin_file = libname_from_la_file(argv[1]);
    else
        plugin_file = g_file_new_for_commandline_arg(argv[1]);

    if( !chimara_glk_run_file(CHIMARA_GLK(glk), plugin_file,
        argc - 1, argv + 1, &error) )
		g_error("Error starting Glk library: %s\n", error->message);
    g_object_unref(plugin_file);

    gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

	chimara_glk_stop(CHIMARA_GLK(glk));
	chimara_glk_wait(CHIMARA_GLK(glk));
	g_object_unref(glk);

	return 0;
}
