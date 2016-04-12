#include <gtk/gtk.h>
#include <libchimara/chimara-glk.h>

/* This is a test program for CSS styling of the Glk program, which is not
implemented so far. */

/* Style the GUI funky */
void
style1(GtkButton *button, GtkStyleProvider *funky_provider)
{
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), funky_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
}

/* Style the GUI nicely */
void
style2(GtkButton *button, GtkStyleProvider *funky_provider)
{
	gtk_style_context_remove_provider_for_screen(gdk_screen_get_default(), funky_provider);
}

int
main(int argc, char **argv)
{
	gtk_init(&argc, &argv);

	/* Create widgets */
	GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget *grid = gtk_grid_new();
	GtkWidget *glk = chimara_glk_new();
	GtkWidget *stylebutton1 = gtk_button_new_with_label("Style 1");
	GtkWidget *stylebutton2 = gtk_button_new_with_label("Style 2");
	GtkCssProvider *funky_provider = gtk_css_provider_new();

	/* Set properties on widgets */
	gtk_widget_set_size_request(win, 400, 400);
	g_object_set(glk, "expand", TRUE, NULL);
	GError *error = NULL;
	gboolean res = gtk_css_provider_load_from_data(funky_provider,
		".glk grid {"
		"  font-size: 14px;"
		"  color: #303030;"
		"  font-family: \"Andale Mono\";"
		"}\n"
		".glk buffer {"
		"  color: #303030;"
		"  font-size: 14px;"
		"  margin-bottom: 5px;"
		"  font-family: \"Book Antiqua\";"
		"}\n"
		".glk buffer.header { font-weight: bold; }\n"
		".glk buffer.alert {"
		"  color: #aa0000;"
		"  font-weight: bold;"
		"}\n"
		".glk buffer.note {"
		"  color: #aaaa00;"
		"  font-weight: bold;"
		"}\n"
		".glk buffer.block-quote {"
		"  /*text-align: center;*/"
		"  font-style: italic;"
		"}\n"
		".glk buffer.input {"
		"  color: #0000aa;"
		"  font-style: italic;"
		"}\n"
		".glk blank { background-color: #4e702a; }\n"
		".glk graphics {"
		"  background-image: -gtk-gradient(linear, 0 0, 0 1,"
		"	color-stop(0, @yellow),"
		"   color-stop(0.2, @blue),"
		"   color-stop(1, #0f0));"
		"}",
		-1, &error);
	if(!res)
		g_printerr("Error: %s\n", error->message);

	/* Put widgets together */
	gtk_grid_attach(GTK_GRID(grid), stylebutton1, 0, 0, 1, 1);
	gtk_grid_attach_next_to(GTK_GRID(grid), stylebutton2, NULL, GTK_POS_RIGHT, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), glk, 0, 1, 2, 1);
	gtk_container_add(GTK_CONTAINER(win), grid);

	/* Connect signals */
	g_signal_connect(win, "delete-event", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(stylebutton1, "clicked", G_CALLBACK(style1), funky_provider);
	g_signal_connect(stylebutton2, "clicked", G_CALLBACK(style2), funky_provider);

	/* Go! */
	gtk_widget_show_all(win);
	g_object_ref(glk);
	char *plugin_argv[] = { "styletest" };
	chimara_glk_run(CHIMARA_GLK(glk), ".libs/styletest.so", 1, plugin_argv, NULL);

	gtk_main();

	chimara_glk_stop(CHIMARA_GLK(glk));
	chimara_glk_wait(CHIMARA_GLK(glk));
	g_object_unref(glk);
	g_object_unref(funky_provider);
	return 0;
}