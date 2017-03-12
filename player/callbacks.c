/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * callbacks.c
 * Copyright (C) Philip en Marijn 2008 <>
 * 
 * callbacks.c is free software copyrighted by Philip en Marijn.
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
 * callbacks.c IS PROVIDED BY Philip en Marijn ``AS IS'' AND ANY EXPRESS
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

#include "error.h"

/* If a game is running in @glk, warn the user that they will quit the currently
running game if they open a new one. Returns TRUE if no game was running.
Returns FALSE if the user cancelled. Returns TRUE and shuts down the running
game if the user wishes to continue. */
static gboolean
confirm_open_new_game(ChimaraGlk *glk)
{
	g_return_val_if_fail(glk && CHIMARA_IS_GLK(glk), FALSE);
	
	GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(glk)));
	
	if(chimara_glk_get_running(glk)) {
		GtkWidget *dialog = gtk_message_dialog_new(window,
		    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		    GTK_MESSAGE_WARNING,
		    GTK_BUTTONS_CANCEL,
		    _("Are you sure you want to open a new game?"));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
		    _("If you open a new game, you will quit the one you are currently playing."));
		gtk_dialog_add_button(GTK_DIALOG(dialog), _("_Open"), GTK_RESPONSE_OK);
		gint response = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		
		if(response != GTK_RESPONSE_OK)
			return FALSE;

		chimara_glk_stop(glk);
		chimara_glk_wait(glk);
	}
	return TRUE;
}

/* Internal function: See if there is a corresponding graphics file */
static void
search_for_graphics_file(const char *filename, ChimaraIF *glk)
{

	extern GSettings *prefs_settings;

	/* First get the name of the story file */
	char *scratch = g_path_get_basename(filename);
	char *ext = strrchr(scratch, '.');
	if(strcmp(ext, ".zlb") == 0 ||
		strcmp(ext, ".zblorb") == 0 ||
		strcmp(ext, ".glb") == 0 ||
		strcmp(ext, ".gblorb") == 0 ||
		strcmp(ext, ".blorb") == 0 ||
		strcmp(ext, ".blb") == 0)
	{
		g_object_set(glk, "graphics-file", NULL, NULL);
		return;
	}
	*ext = '\0';

	/* Check in the stored resource path, if set */
	char *resource_path;
	g_settings_get(prefs_settings, "resource-path", "ms", &resource_path);

	/* Otherwise check in the current directory */
	if(!resource_path)
		resource_path = g_path_get_dirname(filename);

	char *blorbfile = g_strconcat(resource_path, "/", scratch, ".blb", NULL);
	if(g_file_test(blorbfile, G_FILE_TEST_EXISTS))
		g_object_set(glk, "graphics-file", blorbfile, NULL);

	g_free(blorbfile);
	g_free(scratch);
	g_free(resource_path);
}

void
on_open_activate(GSimpleAction *action, GVariant *param, ChimaraGlk *glk)
{
	extern GtkWidget *open_menu;
	GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(glk)));

#if GTK_CHECK_VERSION(3, 22, 0)
	gtk_popover_popdown(GTK_POPOVER(open_menu));
#else
	gtk_widget_hide(open_menu);
#endif

	if(!confirm_open_new_game(glk))
		return;

	GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Open Game"),
	    window,
	    GTK_FILE_CHOOSER_ACTION_OPEN,
	    _("_Cancel"), GTK_RESPONSE_CANCEL,
	    _("_Open"), GTK_RESPONSE_ACCEPT,
	    NULL);

	/* Get last opened path */
	extern GSettings *state_settings;
	gchar *path;
	g_settings_get(state_settings, "last-open-path", "ms", &path);
	if(path) {
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), path);
		g_free(path);
	}

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		GError *error = NULL;
		extern GSettings *prefs_settings;
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		search_for_graphics_file(filename, CHIMARA_IF(glk));
		if(!chimara_if_run_game(CHIMARA_IF(glk), filename, &error)) {
			error_dialog(window, error, _("Could not open game file '%s': "), filename);
			g_free(filename);
			gtk_widget_destroy(dialog);
			return;
		}
		
		path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));
		if(path) {
			g_settings_set(state_settings, "last-open-path", "ms", path);
			g_free(path);
		}

		/* Add file to recent files list */
		GtkRecentManager *manager = gtk_recent_manager_get_default();
		gchar *uri;
		
		if(!(uri = g_filename_to_uri(filename, NULL, &error)))
			g_warning("Could not convert filename '%s' to URI: %s", filename, error->message);
		else {
			if(!gtk_recent_manager_add_item(manager, uri))
				g_warning("Could not add URI '%s' to recent files list.", uri);
			g_free(uri);
		}
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
}

void
on_recent_item_activated(GtkRecentChooser *chooser, ChimaraGlk *glk)
{
	extern GtkWidget *open_menu;

	GError *error = NULL;
	GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(glk)));
	gchar *uri = gtk_recent_chooser_get_current_uri(chooser);
	gchar *filename;

#if GTK_CHECK_VERSION(3, 22, 0)
	gtk_popover_popdown(GTK_POPOVER(open_menu));
#else
	gtk_widget_hide(open_menu);
#endif

	if(!(filename = g_filename_from_uri(uri, NULL, &error))) {
		error_dialog(window, error, _("Could not open game file '%s': "), uri);
		goto finally;
	}
	
	if(!confirm_open_new_game(glk))
		goto finally2;
	
	search_for_graphics_file(filename, CHIMARA_IF(glk));
	if(!chimara_if_run_game(CHIMARA_IF(glk), filename, &error)) {
		error_dialog(window, error, _("Could not open game file '%s': "), filename);
		goto finally2;
	}
	
	/* Add file to recent files list again, this updates it to most recently used */
	GtkRecentManager *manager = gtk_recent_manager_get_default();
	if(!gtk_recent_manager_add_item(manager, uri))
		g_warning("Could not add URI '%s' to recent files list.", uri);

finally2:
	g_free(filename);
finally:
	g_free(uri);
}

void
on_stop_activate(GSimpleAction *action, GVariant *param, ChimaraGlk *glk)
{
	chimara_glk_stop(glk);
}

void
on_quit_chimara_activate(GSimpleAction *action, GVariant *param, ChimaraGlk *glk)
{
	GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(glk));
	gtk_widget_destroy(toplevel);
}

void
on_preferences_activate(GSimpleAction *action, GVariant *param, ChimaraGlk *glk)
{
	extern GtkWidget *prefswindow;
	gtk_window_present(GTK_WINDOW(prefswindow));
}

void
on_undo_activate(GSimpleAction *action, GVariant *value, ChimaraGlk *glk)
{
	chimara_glk_feed_line_input(glk, "undo");
}

void
on_save_activate(GSimpleAction *action, GVariant *value, ChimaraGlk *glk)
{
	chimara_glk_feed_line_input(glk, "save");
}

void
on_restore_activate(GSimpleAction *action, GVariant *value, ChimaraGlk *glk)
{
	chimara_glk_feed_line_input(glk, "restore");
}

void
on_restart_activate(GSimpleAction *action, GVariant *value, ChimaraGlk *glk)
{
	chimara_glk_feed_line_input(glk, "restart");
}

void
on_quit_activate(GSimpleAction *action, GVariant *value, ChimaraGlk *glk)
{
	chimara_glk_feed_line_input(glk, "quit");
}

void
on_about_activate(GtkAction *action, GVariant *value, ChimaraGlk *glk)
{
	extern GtkWidget *aboutwindow;
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(aboutwindow), PACKAGE_VERSION);
	gtk_window_present(GTK_WINDOW(aboutwindow));
}
