#include <glib.h>
#include <libchimara/chimara-glk.h>
#include <libchimara/chimara-if.h>
#include "error.h"

#define LOAD_WIDGET(name) GTK_WIDGET(gtk_builder_get_object(builder, name))

typedef struct {
	GtkWidget *window, *test_picker, *go, *stop, *interp;
} Widgets;

void
on_go_clicked(GtkButton *go, Widgets *w)
{
	GError *error = NULL;

	gchar *filename = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(w->test_picker));
	gtk_combo_box_get_active_iter(GTK_COMBO_BOX(w->test_picker), &iter);
	gtk_tree_model_get(model, &iter, 1, &filename, -1);
	g_object_unref(model);

	gchar *fullpath = g_build_filename(PACKAGE_SRC_DIR, filename, NULL);

	if( !chimara_if_run_game(CHIMARA_IF(w->interp), fullpath, &error) )
	{
		error_dialog(GTK_WINDOW(w->window), error, "Error starting Glk library: ");
		gtk_main_quit();
	}
	g_free(fullpath);
}

void
on_interp_started(ChimaraGlk *glk, Widgets *w)
{
	gtk_widget_set_sensitive(w->go, FALSE);
	gtk_widget_set_sensitive(w->stop, TRUE);
	gtk_widget_set_sensitive(w->test_picker, FALSE);
}

void
on_stop_clicked(GtkButton *stop, Widgets *w)
{
	chimara_glk_stop( CHIMARA_GLK(w->interp) );
	chimara_glk_wait( CHIMARA_GLK(w->interp) );
}

void
on_interp_stopped(ChimaraGlk *glk, Widgets *w)
{
	gtk_widget_set_sensitive(w->stop, FALSE);
	gtk_widget_set_sensitive(w->go, TRUE);
	gtk_widget_set_sensitive(w->test_picker, TRUE);
}

void
on_glulxe_toggled(GtkToggleButton *glulxe, Widgets *w)
{
	if(gtk_toggle_button_get_active(glulxe))
		chimara_if_set_preferred_interpreter(CHIMARA_IF(w->interp), CHIMARA_IF_FORMAT_GLULX, CHIMARA_IF_INTERPRETER_GLULXE);
}

void
on_git_toggled(GtkToggleButton *git, Widgets *w)
{
	if(gtk_toggle_button_get_active(git))
		chimara_if_set_preferred_interpreter(CHIMARA_IF(w->interp), CHIMARA_IF_FORMAT_GLULX, CHIMARA_IF_INTERPRETER_GIT);
}

int
main(int argc, char *argv[])
{
	GError *error = NULL;

	if( !g_thread_supported() )
		g_thread_init(NULL);
	gdk_threads_init();
	gtk_init(&argc, &argv);

	GtkBuilder *builder = gtk_builder_new();
	if(!gtk_builder_add_from_file(builder, PACKAGE_SRC_DIR "/glulxercise.ui", &error))
	{
		error_dialog(NULL, error, "Failed to build interface: ");
		return 1;
	}

	Widgets *w = g_slice_new0(Widgets);
	w->window = LOAD_WIDGET("window");
	GtkWidget *vbox = LOAD_WIDGET("vbox");
	w->test_picker = LOAD_WIDGET("test_picker");
	gtk_combo_box_set_active(GTK_COMBO_BOX(w->test_picker), 0);
	w->go = LOAD_WIDGET("go");
	w->stop = LOAD_WIDGET("stop");
	w->interp = chimara_if_new();
	gtk_widget_set_size_request(w->interp, 500, 600);
	gtk_box_pack_end_defaults(GTK_BOX(vbox), w->interp);
	gtk_builder_connect_signals(builder, w);
	g_signal_connect(w->interp, "started", G_CALLBACK(on_interp_started), w);
	g_signal_connect(w->interp, "stopped", G_CALLBACK(on_interp_stopped), w);
	gtk_widget_show_all(w->window);

	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

	chimara_glk_stop( CHIMARA_GLK(w->interp) );
	chimara_glk_wait( CHIMARA_GLK(w->interp) );

	g_slice_free(Widgets, w);

	return 0;
}
