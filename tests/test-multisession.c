/* Program for testing multisessionality, i.e. whether two ChimaraGlk widgets
 can run in the same application. */

#include <glib.h>
#include <gtk/gtk.h>
#include <libchimara/chimara-glk.h>

static void
on_started(ChimaraGlk *glk, const gchar *data)
{
    g_printerr("%s started!\n", data);
}

static void
on_stopped(ChimaraGlk *glk, const gchar *data)
{
    g_printerr("%s stopped!\n", data);
}

static gboolean
on_delete_event(void)
{
	gtk_main_quit();
	return TRUE;
}

int
main(int argc, char **argv)
{
	gdk_threads_init();
	gtk_init(&argc, &argv);

	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request(window, 800, 500);
	g_signal_connect(window, "delete_event", G_CALLBACK(on_delete_event), NULL);
	
	GtkWidget *hpaned = gtk_hpaned_new();
	gtk_paned_set_position(GTK_PANED(hpaned), 400);
	
	GtkWidget *frotz = chimara_glk_new();
	chimara_glk_set_css_from_string(CHIMARA_GLK(frotz),
	    "buffer.normal { font-family: 'Lucida Sans'; font-size: 12; }"
	    "grid.normal { font-family: 'Lucida Console'; font-size: 12; }");
	g_signal_connect(frotz, "started", G_CALLBACK(on_started), "Frotz");
	g_signal_connect(frotz, "stopped", G_CALLBACK(on_stopped), "Frotz");
	
	GtkWidget *nitfol = chimara_glk_new();
	chimara_glk_set_css_from_string(CHIMARA_GLK(frotz),
	    "buffer.normal { font-family: 'Bitstream Charter'; font-size: 12; }"
	    "grid.normal { font-family: 'Luxi Mono'; font-size: 12; }");
	g_signal_connect(nitfol, "started", G_CALLBACK(on_started), "Nitfol");
	g_signal_connect(nitfol, "stopped", G_CALLBACK(on_stopped), "Nitfol");

	gtk_paned_pack1(GTK_PANED(hpaned), frotz, TRUE, TRUE);
	gtk_paned_pack2(GTK_PANED(hpaned), nitfol, TRUE, TRUE);
	gtk_container_add(GTK_CONTAINER(window), hpaned);

	gtk_widget_show_all(window);

	if(!chimara_glk_run(CHIMARA_GLK(frotz), "../interpreters/frotz/.libs/frotz.so", argc, argv, NULL))
		return 1;
	if(!chimara_glk_run(CHIMARA_GLK(nitfol), "../interpreters/nitfol/.libs/nitfol.so", argc, argv, NULL))
		return 1;
	
    gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
	
	chimara_glk_stop(CHIMARA_GLK(frotz));
	chimara_glk_stop(CHIMARA_GLK(nitfol));

	return 0;
}
	
	