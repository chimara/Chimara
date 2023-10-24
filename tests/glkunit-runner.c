#include <gtk/gtk.h>
#include <libchimara/chimara-glk.h>

static char **char_input_queue = NULL;
static size_t char_input_ix = 0;
static unsigned handler_id = 0;
static GOptionEntry option_entries[] = {
	{ "char-input", 'c', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING_ARRAY, &char_input_queue, "Character input", "CHAR_NAME" },
	{ NULL },
};

extern unsigned char_input_specifier_to_keysym(const char *specifier);

static gboolean
feed_input(ChimaraGlk *glk)
{
	if (!char_input_queue || char_input_ix >= g_strv_length(char_input_queue)) {
		handler_id = 0;
		return G_SOURCE_REMOVE;
	}

	unsigned keyval = char_input_specifier_to_keysym(char_input_queue[char_input_ix]);
	chimara_glk_feed_char_input(glk, keyval);

	char_input_ix++;
	return G_SOURCE_CONTINUE;
}

int
main(int argc, char *argv[])
{
	GError *error = NULL;

	gtk_init(&argc, &argv);

	g_autoptr(GOptionContext) options = g_option_context_new("PLUGIN");
	g_option_context_add_main_entries(options, option_entries, NULL);
	if (!g_option_context_parse(options, &argc, &argv, &error))
		g_error("Unrecognized argument %s", error->message);

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

	handler_id = g_timeout_add(100, (GSourceFunc)feed_input, glk);

	gtk_main();

	g_clear_handle_id(&handler_id, g_source_remove);

	chimara_glk_stop(CHIMARA_GLK(glk));
	chimara_glk_wait(CHIMARA_GLK(glk));

	gtk_widget_destroy(win);

	return 0;
}
