/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 * Copyright (C) Philip en Marijn 2008 <>
 * 
 * main.c is free software copyrighted by Philip en Marijn.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name ``Philip en Marijn'' nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 
 * main.c IS PROVIDED BY Philip en Marijn ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Philip en Marijn OR ANY OTHER CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libchimara/chimara-glk.h>
#include <libchimara/chimara-if.h>

#include "actions.h"
#include "error.h"
#include "preferences.h"

/* Static global pointers */
static GtkApplication *app = NULL;
static GtkWidget *window = NULL;
static GtkWidget *glk = NULL;

/* Global global pointers */
GtkWidget *aboutwindow = NULL;
GtkWidget *prefswindow = NULL;
GtkWidget *open_menu = NULL;
GSettings *prefs_settings = NULL;
GSettings *state_settings = NULL;

GObject *
load_object(GtkBuilder *builder, const gchar *name)
{
	GObject *retval;
	if( (retval = gtk_builder_get_object(builder, name)) == NULL) {
		error_dialog(NULL, NULL, "Error while getting object '%s'", name);
		g_error("Error while getting object '%s'", name);
	}
	return retval;
}

static void
change_window_title(ChimaraGlk *glk, GParamSpec *pspec, GtkHeaderBar *titlebar)
{
	char *program_name, *story_name;
	g_object_get(glk, "program-name", &program_name, "story-name", &story_name, NULL);
	gtk_header_bar_set_title(titlebar, story_name ? story_name : _("Chimara"));
	gtk_header_bar_set_subtitle(titlebar, program_name ? program_name : _("Interactive Fiction Player"));
	g_free(program_name);
	g_free(story_name);
}

static gboolean
create_window(void)
{
	GError *error = NULL;

	GtkBuilder *builder = gtk_builder_new_from_resource("/org/chimara-if/player/chimara.ui");
	window = GTK_WIDGET(load_object(builder, "chimara"));
	aboutwindow = GTK_WIDGET(load_object(builder, "aboutwindow"));
	prefswindow = GTK_WIDGET(load_object(builder, "prefswindow"));

	glk = chimara_if_new();
	g_object_set(glk,
	    "ignore-errors", TRUE,
	    /*"interpreter-number", CHIMARA_IF_ZMACHINE_TANDY_COLOR,*/
	    NULL);

	GBytes *css_bytes = g_resources_lookup_data("/org/chimara-if/player/style.css",
		G_RESOURCE_LOOKUP_FLAGS_NONE, &error);
	if (!css_bytes)
		return FALSE;

	size_t len;
	char *css = g_bytes_unref_to_data(css_bytes, &len);
	chimara_glk_set_css_from_string(CHIMARA_GLK(glk), css);
	g_free(css);

	/* DON'T UNCOMMENT THIS your eyes will burn
	 but it is a good test of programmatically altering just one style
	chimara_glk_set_css_from_string(CHIMARA_GLK(glk),
	    "buffer { font-family: 'Comic Sans MS'; }");*/

	create_app_actions(G_ACTION_MAP(app), glk);
	create_window_actions(G_ACTION_MAP(window), glk);

	GtkWidget *hamburger_button = GTK_WIDGET(load_object(builder, "hamburger_button"));
	gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(hamburger_button),
		G_MENU_MODEL(gtk_application_get_menu_by_id(app, "hamburger-menu")));

	GtkWidget *open_button = GTK_WIDGET(load_object(builder, "open_button"));
	open_menu = GTK_WIDGET(load_object(builder, "open_menu"));
	gtk_menu_button_set_popover(GTK_MENU_BUTTON(open_button), GTK_WIDGET(open_menu));

	gtk_container_add(GTK_CONTAINER(window), glk);

	gtk_builder_connect_signals(builder, glk);
	GtkWidget *titlebar = GTK_WIDGET(load_object(builder, "titlebar"));
	g_signal_connect(glk, "notify::program-name", G_CALLBACK(change_window_title), titlebar);
	g_signal_connect(glk, "notify::story-name", G_CALLBACK(change_window_title), titlebar);
	
	/* Create preferences window */
	preferences_create(builder, CHIMARA_GLK(glk));

	g_object_unref(builder);

	g_object_ref(glk);

	return TRUE;
}

static void
on_startup(GApplication *gapp)
{
	if( !create_window() ) {
		error_dialog(NULL, NULL, "Error while building interface.");
		g_error("Error while building interface.");
	}
}

static void
on_activate(GApplication *gapp)
{
	gtk_application_add_window(app, GTK_WINDOW(window));
	gtk_widget_show_all(window);
	gtk_window_present(GTK_WINDOW(window));
}

static void
on_open(GApplication *gapp, GFile **files, int n_files, char *hint)
{
	GError *error = NULL;

	if(n_files == 2) {
		char *graphics_file_path = g_file_get_path(files[1]);
		g_object_set(glk, "graphics-file", graphics_file_path, NULL);
		g_free(graphics_file_path);
	}
	if(n_files >= 1) {
		if( !chimara_if_run_game_file(CHIMARA_IF(glk), files[0], &error) ) {
			error_dialog(GTK_WINDOW(window), error, "Error starting Glk library: ");
			g_application_quit(gapp);
		}
	}
	on_activate(gapp);
}

int
main(int argc, char *argv[])
{
#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
#endif

	gtk_init(&argc, &argv);

	prefs_settings = g_settings_new("org.chimara-if.player.preferences");
	state_settings = g_settings_new("org.chimara-if.player.state");

	app = gtk_application_new("org.chimara-if.player", G_APPLICATION_HANDLES_OPEN);
	g_signal_connect(app, "startup", G_CALLBACK(on_startup), NULL);
	g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
	g_signal_connect(app, "open", G_CALLBACK(on_open), NULL);

	int status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	chimara_glk_stop(CHIMARA_GLK(glk));
	chimara_glk_wait(CHIMARA_GLK(glk));

	g_object_unref(glk);

	return status;
}
