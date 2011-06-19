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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

/* Use a custom GSettings backend for our preferences file */
#define G_SETTINGS_ENABLE_BACKEND
#include <gio/gsettingsbackend.h>

#include "error.h"
#include <libchimara/chimara-glk.h>
#include <libchimara/chimara-if.h>

#include "preferences.h"
#include "player.h"



/* Global global pointers */
GtkBuilder *builder = NULL;
GtkWidget *aboutwindow = NULL;
GtkWidget *prefswindow = NULL;

GSettings *prefs_settings = NULL;
GSettings *state_settings = NULL;

static GObject *
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
			error_dialog(NULL, error, "Error while building interface: ");	
			return;
#ifdef DEBUG
		}
#endif /* DEBUG */
	}

	aboutwindow = GTK_WIDGET(load_object("aboutwindow"));
	prefswindow = GTK_WIDGET(load_object("prefswindow"));

	const gchar **ptr;
	GtkRecentFilter *filter = gtk_recent_filter_new();
	/* TODO: Use mimetypes and construct the filter dynamically depending on 
	what plugins are installed */
	const gchar *patterns[] = {
		"*.z[1-8]", "*.[zg]lb", "*.[zg]blorb", "*.ulx", "*.blb", "*.blorb", NULL
	};

	for(ptr = patterns; *ptr; ptr++)
		gtk_recent_filter_add_pattern(filter, *ptr);
	GtkRecentChooser *recent = GTK_RECENT_CHOOSER(load_object("recent"));
	gtk_recent_chooser_add_filter(recent, filter);

	/* Create preferences window */
	preferences_create();
}

int
main(int argc, char *argv[])
{
	GError *error = NULL;

#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
#endif

	if( !g_thread_supported() )
		g_thread_init(NULL);
	gdk_threads_init();
	gtk_init(&argc, &argv);

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
	prefs_settings = g_settings_new_with_backend("org.chimara-if.player.preferences", backend);
	state_settings = g_settings_new_with_backend("org.chimara-if.player.state", backend);
	g_free(keyfile);

	create_window();

	GtkWidget *window = chimara_player_new();
	gtk_widget_show_all(window);

	//if(argc == 3) {
	//	g_object_set(glk, "graphics-file", argv[2], NULL);
	//}
	//if(argc >= 2) {
	//	if( !chimara_if_run_game(CHIMARA_IF(glk), argv[1], &error) ) {
	//   		error_dialog(GTK_WINDOW(window), error, "Error starting Glk library: ");
	//		return 1;
	//	}
	//}

    gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

	return 0;
}
