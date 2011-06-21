/*
 * Copyright (C) 2008, 2009, 2010, 2011 Philip Chimento and Marijn van Vliet.
 * All rights reserved.
 *
 * Chimara is free software copyrighted by Philip Chimento and Marijn van Vliet.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither of the names Philip Chimento or Marijn van Vliet, nor the name of
 *    any other contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

/* Use a custom GSettings backend for our preferences file */
#define G_SETTINGS_ENABLE_BACKEND
#include <gio/gsettingsbackend.h>

#include <config.h>
#include <libchimara/chimara-if.h>
#include "app.h"
#include "browser.h"
#include "error.h"
#include "player.h"
#include "preferences.h"
#include "util.h"

typedef struct _ChimaraAppPrivate {
	GtkActionGroup *action_group;
} ChimaraAppPrivate;

#define CHIMARA_APP_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), CHIMARA_TYPE_APP, ChimaraAppPrivate))
#define CHIMARA_APP_USE_PRIVATE ChimaraAppPrivate *priv = CHIMARA_APP_PRIVATE(self)

G_DEFINE_TYPE(ChimaraApp, chimara_app, G_TYPE_OBJECT);

static void
chimara_app_finalize(GObject *self)
{
	CHIMARA_APP_USE_PRIVATE;
	g_object_unref(priv->action_group);
	
	/* Chain up */
	G_OBJECT_CLASS(chimara_app_parent_class)->finalize(self);
}

static void
chimara_app_class_init(ChimaraAppClass *klass)
{
	/* Override methods of parent classes */
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = chimara_app_finalize;

	/* Private data */
	g_type_class_add_private(klass, sizeof(ChimaraAppPrivate));
}

static void
chimara_app_init(ChimaraApp *self)
{
	CHIMARA_APP_USE_PRIVATE;

	/* Create configuration dir ~/.chimara */
	gchar *configdir = g_build_filename(g_get_home_dir(), ".chimara", NULL);
	if(!g_file_test(configdir, G_FILE_TEST_IS_DIR)
		&& g_mkdir(configdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
		g_error(_("Cannot create configuration directory ~/.chimara"));
	g_free(configdir);

	/* Initialize settings file; it can be overridden by a "chimara-config" file
	 in the current directory */
	gchar *keyfile;
	if(g_file_test("chimara-config", G_FILE_TEST_IS_REGULAR))
		keyfile = g_strdup("chimara-config");
	else
		keyfile = g_build_filename(g_get_home_dir(), ".chimara", "config", NULL);
	GSettingsBackend *backend = g_keyfile_settings_backend_new(keyfile, "/org/chimara-if/player/", NULL);
	self->prefs_settings = g_settings_new_with_backend("org.chimara-if.player.preferences", backend);
	self->state_settings = g_settings_new_with_backend("org.chimara-if.player.state", backend);
	g_free(keyfile);

	/* Build user interface */
	char *object_ids[] = {
		"app_group",
		"aboutwindow",
		NULL
	};
	GtkBuilder *builder = new_builder_with_objects(object_ids);

	self->aboutwindow = GTK_WIDGET(load_object(builder, "aboutwindow"));
	priv->action_group = GTK_ACTION_GROUP(load_object(builder, "app_group"));
	g_object_ref(priv->action_group);

	const gchar **ptr;
	GtkRecentFilter *filter = gtk_recent_filter_new();
	/* TODO: Use mimetypes and construct the filter dynamically depending on 
	what plugins are installed */
	const gchar *patterns[] = {
		"*.z[1-8]", "*.[zg]lb", "*.[zg]blorb", "*.ulx", "*.blb", "*.blorb", NULL
	};

	for(ptr = patterns; *ptr; ptr++)
		gtk_recent_filter_add_pattern(filter, *ptr);
	GtkRecentChooser *recent = GTK_RECENT_CHOOSER(load_object(builder, "recent"));
	gtk_recent_chooser_add_filter(recent, filter);

	gtk_builder_connect_signals(builder, self);

	g_object_unref(builder);
}

/* PUBLIC FUNCTIONS */

ChimaraApp *
chimara_app_get(void)
{
    static ChimaraApp *theapp = NULL;

    if(G_UNLIKELY(theapp == NULL)) {
    		theapp = CHIMARA_APP(g_object_new(CHIMARA_TYPE_APP, NULL));

		/* Create one-per-application windows */
		theapp->prefswindow = chimara_prefs_new();
		theapp->browser_window = chimara_browser_new();
	}

    	return theapp;
}

GtkActionGroup *
chimara_app_get_action_group(ChimaraApp *self)
{
	CHIMARA_APP_USE_PRIVATE;
	return priv->action_group;
}

/* GLADE CALLBACKS */

/* Internal function: See if there is a corresponding graphics file. If so,
return its path. If not, return NULL. */
static char *
search_for_graphics_file(const char *filename)
{
	ChimaraApp *theapp = chimara_app_get();

	/* First get the name of the story file */
	char *scratch = g_path_get_basename(filename);
	*(strrchr(scratch, '.')) = '\0';

	/* Check in the stored resource path, if set */
	char *resource_path;
	g_settings_get(theapp->prefs_settings, "resource-path", "ms", &resource_path);

	/* Otherwise check in the current directory */
	if(!resource_path)
		resource_path = g_path_get_dirname(filename);

	char *blorbfile = g_strconcat(resource_path, "/", scratch, ".blb", NULL);
	g_free(scratch);
	g_free(resource_path);

	if(g_file_test(blorbfile, G_FILE_TEST_EXISTS))
		return blorbfile;

	g_free(blorbfile);
	return NULL;
}

void
on_open_activate(GtkAction *action, ChimaraApp *theapp)
{
	//if(!confirm_open_new_game(CHIMARA_GLK(player->glk)))
	//	return;

	GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Open Game"),
	    NULL, // FIXME
	    GTK_FILE_CHOOSER_ACTION_OPEN,
	    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
	    NULL);

	/* Get last opened path */
	gchar *path;
	g_settings_get(theapp->state_settings, "last-open-path", "ms", &path);
	if(path) {
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), path);
		g_free(path);
	}

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		GError *error = NULL;
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		/* Open a new player window */
		ChimaraPlayer *player = CHIMARA_PLAYER(chimara_player_new());
		gtk_widget_show_all(GTK_WIDGET(player));
		gtk_window_present(GTK_WINDOW(player));

		gchar *blorbfile = search_for_graphics_file(filename);
		if(blorbfile) {
			g_object_set(player->glk, "graphics-file", blorbfile, NULL);
			g_free(blorbfile);
		}
		if(!chimara_if_run_game(CHIMARA_IF(player->glk), filename, &error)) {
			error_dialog(GTK_WINDOW(player), error, _("Could not open game file '%s': "), filename);
			g_free(filename);
			gtk_widget_destroy(dialog);
			return;
		}
		
		path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));
		if(path) {
			g_settings_set(theapp->state_settings, "last-open-path", "ms", path);
			g_free(path);
		}

		/* Add file to recent files list */
		GtkRecentManager *manager = gtk_recent_manager_get_default();
		gchar *uri;
		
		if(!(uri = g_filename_to_uri(filename, NULL, &error)))
			g_warning(_("Could not convert filename '%s' to URI: %s"), filename, error->message);
		else {
			if(!gtk_recent_manager_add_item(manager, uri))
				g_warning(_("Could not add URI '%s' to recent files list."), uri);
			g_free(uri);
		}
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
}

void
on_recent_item_activated(GtkRecentChooser *chooser, ChimaraApp *theapp)
{
	GError *error = NULL;
	gchar *uri = gtk_recent_chooser_get_current_uri(chooser);
	gchar *filename;
	if(!(filename = g_filename_from_uri(uri, NULL, &error))) {
		error_dialog(NULL /* FIXME */, error, _("Could not open game file '%s': "), uri);
		goto finally;
	}
	
	//if(!confirm_open_new_game(CHIMARA_GLK(player->glk)))
	//	goto finally2;

	/* Open a new player window */
	ChimaraPlayer *player = CHIMARA_PLAYER(chimara_player_new());
	gtk_widget_show_all(GTK_WIDGET(player));
	gtk_window_present(GTK_WINDOW(player));
	
	char *blorbfile = search_for_graphics_file(filename);
	if(blorbfile) {
		g_object_set(player->glk, "graphics-file", blorbfile, NULL);
		g_free(blorbfile);
	}
	if(!chimara_if_run_game(CHIMARA_IF(player->glk), filename, &error)) {
		error_dialog(GTK_WINDOW(player), error, _("Could not open game file '%s': "), filename);
		goto finally2;
	}
	
	/* Add file to recent files list again, this updates it to most recently used */
	GtkRecentManager *manager = gtk_recent_manager_get_default();
	if(!gtk_recent_manager_add_item(manager, uri))
		g_warning(_("Could not add URI '%s' to recent files list."), uri);

finally2:
	g_free(filename);
finally:
	g_free(uri);
}

void 
on_quit_chimara_activate(GtkAction *action, ChimaraApp *theapp)
{
	gtk_main_quit();
}

void
on_preferences_activate(GtkAction *action, ChimaraApp *theapp)
{
	gtk_window_present(GTK_WINDOW(theapp->prefswindow));
}

void
on_about_activate(GtkAction *action, ChimaraApp *theapp)
{
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(theapp->aboutwindow), PACKAGE_VERSION);
	gtk_window_present(GTK_WINDOW(theapp->aboutwindow));
}

