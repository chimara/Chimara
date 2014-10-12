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

#include <sys/stat.h>

#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <libchimara/chimara-glk.h>
#include <libchimara/chimara-if.h>

/* Use a custom GSettings backend for our preferences file */
#define G_SETTINGS_ENABLE_BACKEND
#include <gio/gsettingsbackend.h>

#include "actions.h"
#include "error.h"
#include "preferences.h"

/* Static global pointers */
static GtkApplication *app = NULL;
static GtkWidget *window = NULL;
static GtkWidget *glk = NULL;

/* Global global pointers */
GtkBuilder *builder = NULL;
GtkWidget *aboutwindow = NULL;
GtkWidget *prefswindow = NULL;
GtkWidget *recentwindow = NULL;
GSettings *prefs_settings = NULL;
GSettings *state_settings = NULL;

GObject *
load_object(const gchar *name)
{
	GObject *retval;
	if( (retval = gtk_builder_get_object(builder, name)) == NULL) {
		error_dialog(NULL, NULL, "Error while getting object '%s'", name);
		g_error("Error while getting object '%s'", name);
	}
	return retval;
}

static void
change_window_title(ChimaraGlk *glk, GParamSpec *pspec, GtkWindow *window)
{
	gchar *program_name, *story_name, *title;
	g_object_get(glk, "program-name", &program_name, "story-name", &story_name, NULL);
	if(!program_name) {
		gtk_window_set_title(window, "Chimara");
		return;
	}
	else if(!story_name)
		title = g_strdup_printf("%s - Chimara", program_name);
	else
		title = g_strdup_printf("%s - %s - Chimara", program_name, story_name);
		
	g_free(program_name);
	g_free(story_name);
	gtk_window_set_title(window, title);
	g_free(title);
}

static gboolean
create_window(void)
{
	GError *error = NULL;

   	builder = gtk_builder_new();
	if( !gtk_builder_add_from_file(builder, PACKAGE_DATA_DIR "/chimara.ui", &error) ) {
#ifdef DEBUG
		g_error_free(error);
		error = NULL;
		if( !gtk_builder_add_from_file(builder, PACKAGE_SRC_DIR "/chimara.ui", &error) ) {
#endif /* DEBUG */
			return FALSE;
#ifdef DEBUG
		}
#endif /* DEBUG */
	}

	window = GTK_WIDGET(load_object("chimara"));
	aboutwindow = GTK_WIDGET(load_object("aboutwindow"));
	prefswindow = GTK_WIDGET(load_object("prefswindow"));
	recentwindow = GTK_WIDGET(load_object("recentwindow"));

	glk = chimara_if_new();
	g_object_set(glk,
	    "ignore-errors", TRUE,
	    /*"interpreter-number", CHIMARA_IF_ZMACHINE_TANDY_COLOR,*/
	    NULL);
	if( !chimara_glk_set_css_from_file(CHIMARA_GLK(glk), PACKAGE_DATA_DIR "/style.css", &error) ) {
#ifdef DEBUG
		g_error_free(error);
		error = NULL;
		if( !chimara_glk_set_css_from_file(CHIMARA_GLK(glk), PACKAGE_SRC_DIR "/style.css", &error) )
#endif /* DEBUG */
			return FALSE;
	}
	
	/* DON'T UNCOMMENT THIS your eyes will burn
	 but it is a good test of programmatically altering just one style
	chimara_glk_set_css_from_string(CHIMARA_GLK(glk),
	    "buffer { font-family: 'Comic Sans MS'; }");*/
	
	GtkBox *vbox = GTK_BOX( gtk_builder_get_object(builder, "vbox") );			
	if(vbox == NULL)
		return FALSE;

	create_app_actions(G_ACTION_MAP(app), glk);
	create_window_actions(G_ACTION_MAP(window), glk);

	GtkWidget *toolbar = GTK_WIDGET(gtk_builder_get_object(builder, "toolbar"));

	/* Set the default value of the "View/Toolbar" menu item upon creation of a
	 new window to the "show-toolbar-default" setting, but bind the setting
	 one-way only - we don't want toolbars to disappear suddenly */
	GPropertyAction *toolbar_action = g_property_action_new("toolbar", toolbar, "visible");
	g_action_map_add_action(G_ACTION_MAP(window), G_ACTION(toolbar_action));
	if (g_settings_get_boolean(state_settings, "show-toolbar-default"))
		gtk_widget_show(toolbar);
	else
		gtk_widget_hide(toolbar);
	g_settings_bind(state_settings, "show-toolbar-default",
		toolbar, "visible", G_SETTINGS_BIND_SET);

	GMenuModel *menumodel = G_MENU_MODEL(gtk_builder_get_object(builder, "menubar"));
	GtkWidget *menubar = gtk_menu_bar_new_from_model(menumodel);
	g_object_unref(menumodel);

	gtk_box_pack_end(vbox, glk, TRUE, TRUE, 0);
	gtk_box_pack_start(vbox, menubar, FALSE, FALSE, 0);
	gtk_box_pack_start(vbox, toolbar, FALSE, FALSE, 0);

	gtk_builder_connect_signals(builder, glk);
	g_signal_connect(glk, "notify::program-name", G_CALLBACK(change_window_title), window);
	g_signal_connect(glk, "notify::story-name", G_CALLBACK(change_window_title), window);
	
	/* Create preferences window */
	preferences_create(CHIMARA_GLK(glk));

	return TRUE;
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

	/* Create configuration dir ~/.chimara */
	gchar *configdir = g_build_filename(g_get_home_dir(), ".chimara", NULL);
	if(!g_file_test(configdir, G_FILE_TEST_IS_DIR)
		&& g_mkdir(configdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
		g_error("Cannot create configuration directory ~/.chimara");
	g_free(configdir);

	/* Initialize settings file; it can be overridden by a "chimara-config" file
	 in the current directory */
	gchar *keyfile;
	if(g_file_test("chimara-config", G_FILE_TEST_IS_REGULAR))
		keyfile = g_strdup("chimara-config");
	else
		keyfile = g_build_filename(g_get_home_dir(), ".chimara", "config", NULL);
	GSettingsBackend *backend = g_keyfile_settings_backend_new(keyfile, "/org/chimara-if/player/", NULL);
	prefs_settings = g_settings_new_with_backend("org.chimara-if.player.preferences", backend);
	state_settings = g_settings_new_with_backend("org.chimara-if.player.state", backend);
	g_free(keyfile);

	app = gtk_application_new("org.chimara-if.player", G_APPLICATION_HANDLES_OPEN);
	g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
	g_signal_connect(app, "open", G_CALLBACK(on_open), NULL);

	if( !create_window() ) {
		error_dialog(NULL, NULL, "Error while building interface.");
		return 1;
	}

	int status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	chimara_glk_stop(CHIMARA_GLK(glk));
	chimara_glk_wait(CHIMARA_GLK(glk));

	g_object_unref( G_OBJECT(builder) );

	return status;
}
