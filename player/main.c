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
#include <gtk/gtk.h>

#include "error.h"
#include <libchimara/chimara-glk.h>
#include <libchimara/chimara-if.h>

/* Static global pointers to widgets */
static GtkBuilder *builder = NULL;
static GtkUIManager *uimanager = NULL;
static GtkWidget *window = NULL;
static GtkWidget *glk = NULL;

/* Global global pointers */
GtkWidget *aboutwindow = NULL;
GtkWidget *prefswindow = NULL;

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
	if( !gtk_builder_add_from_file(builder, PACKAGE_SRC_DIR "/chimara.ui", &error) ) {
		error_dialog(NULL, error, "Error while building interface: ");	
		return;
	}

	window = GTK_WIDGET(load_object("chimara"));
	aboutwindow = GTK_WIDGET(load_object("aboutwindow"));
	prefswindow = GTK_WIDGET(load_object("prefswindow"));
	GtkActionGroup *actiongroup = GTK_ACTION_GROUP(load_object("actiongroup"));

	/* Add all the actions to the action group. This for-loop is a temporary fix
	and can be removed once Glade supports adding actions and accelerators to an
	action group. */
	const gchar *actions[] = { 
		"game", "",
		"open", "<ctrl>O", 
		"recent", "",
		"stop", "",
		"quit_chimara", NULL, /* NULL means use stock accelerator */
		"command", "",
		"undo", "<ctrl>Z",
		"save", NULL, 
		"restore", "<ctrl>R", 
		"restart", "",
		"quit", "",
		"edit", "",
		"copy", NULL,
		"paste", NULL,
		"preferences", "",
		"help", "",
		"about", "",
		NULL
	};
	const gchar **ptr;
	for(ptr = actions; *ptr; ptr += 2)
		gtk_action_group_add_action_with_accel(actiongroup, GTK_ACTION(load_object(ptr[0])), ptr[1]);
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

	uimanager = gtk_ui_manager_new();
	if( !gtk_ui_manager_add_ui_from_file(uimanager, PACKAGE_SRC_DIR "/chimara.menus", &error) ) {
		error_dialog(NULL, error, "Error while building interface: ");
		return;
	}
	
	glk = chimara_if_new();
	g_object_set(glk, "ignore-errors", TRUE, NULL);
	
	GtkBox *vbox = GTK_BOX( gtk_builder_get_object(builder, "vbox") );			
	if(vbox == NULL)
	{
		error_dialog(NULL, NULL, "Could not find vbox");
		return;
	}

	gtk_ui_manager_insert_action_group(uimanager, actiongroup, 0);
	GtkWidget *menubar = gtk_ui_manager_get_widget(uimanager, "/menubar");
	GtkWidget *toolbar = gtk_ui_manager_get_widget(uimanager, "/toolbar");

	gtk_box_pack_end(vbox, glk, TRUE, TRUE, 0);
	gtk_box_pack_start(vbox, menubar, FALSE, FALSE, 0);
	gtk_box_pack_start(vbox, toolbar, FALSE, FALSE, 0);
	
	gtk_builder_connect_signals(builder, glk);
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

	create_window();
	gtk_widget_show_all(window);

	g_object_unref( G_OBJECT(builder) );
	g_object_unref( G_OBJECT(uimanager) );

	if(argc >= 2) {
		if( !chimara_if_run_game(CHIMARA_IF(glk), argv[1], &error) ) {
	   		error_dialog(GTK_WINDOW(window), error, "Error starting Glk library: ");
			return 1;
		}
	}

    gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

	chimara_glk_stop(CHIMARA_GLK(glk));
	chimara_glk_wait(CHIMARA_GLK(glk));

	return 0;
}
