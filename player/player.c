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

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libchimara/chimara-glk.h>
#include <libchimara/chimara-if.h>
#include "player.h"
#include "app.h"
#include "error.h"
#include "util.h"

typedef struct _ChimaraPlayerPrivate {
	int dummy;
} ChimaraPlayerPrivate;

#define CHIMARA_PLAYER_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), CHIMARA_TYPE_PLAYER, ChimaraPlayerPrivate))
#define CHIMARA_PLAYER_USE_PRIVATE ChimaraPlayerPrivate *priv = CHIMARA_PLAYER_PRIVATE(self)

G_DEFINE_TYPE(ChimaraPlayer, chimara_player, GTK_TYPE_WINDOW);

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

static void
chimara_player_dispose(GObject *object)
{
	ChimaraPlayer *self = CHIMARA_PLAYER(object);
	if(chimara_glk_get_running(CHIMARA_GLK(self->glk))) {
		chimara_glk_stop(CHIMARA_GLK(self->glk));
		chimara_glk_wait(CHIMARA_GLK(self->glk));
	}
	
	/* Chain up */
	G_OBJECT_CLASS(chimara_player_parent_class)->dispose(object);
}

static void
chimara_player_finalize(GObject *object)
{
	g_object_unref(CHIMARA_PLAYER(object)->glk);
	
	/* Chain up */
	G_OBJECT_CLASS(chimara_player_parent_class)->finalize(object);
}

static void
chimara_player_class_init(ChimaraPlayerClass *klass)
{
	/* Override methods of parent classes */
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	object_class->dispose = chimara_player_dispose;
	object_class->finalize = chimara_player_finalize;

	/* Private data */
	g_type_class_add_private(klass, sizeof(ChimaraPlayerPrivate));
}

static void
chimara_player_init(ChimaraPlayer *self)
{	
	GError *error = NULL;
	ChimaraApp *theapp = chimara_app_get();

	/* Set parent properties */
	g_object_set(self,
		"title", _("Chimara"),
		"default-width", 600,
		"default-height", 800,
		NULL);

	/* Construct user interface */
	char *object_ids[] = {
		"actiongroup",
		"player-vbox",
		NULL
	};
	GtkBuilder *builder = new_builder_with_objects(object_ids);

	GtkActionGroup *actiongroup = GTK_ACTION_GROUP(load_object(builder, "actiongroup"));

	/* Set the default value of the "View/Toolbar" menu item upon creation of a
	 new window to the "show-toolbar-default" setting, but bind the setting
	 one-way only - we don't want toolbars to disappear suddenly */
	GtkToggleAction *toolbar_action = GTK_TOGGLE_ACTION(load_object(builder, "toolbar"));
	gtk_toggle_action_set_active(toolbar_action, g_settings_get_boolean(theapp->state_settings, "show-toolbar-default"));
	g_settings_bind(theapp->state_settings, "show-toolbar-default", toolbar_action, "active", G_SETTINGS_BIND_SET);

	self->glk = chimara_if_new();
	g_object_set(self->glk,
				 "ignore-errors", TRUE,
				 /*"interpreter-number", CHIMARA_IF_ZMACHINE_TANDY_COLOR,*/
				 NULL);
	char *default_css = get_data_file_path("style.css");
	if( !chimara_glk_set_css_from_file(CHIMARA_GLK(self->glk), default_css, &error) ) {
		error_dialog(GTK_WINDOW(self), error, "Couldn't open default CSS file: ");
	}
	
	/* DON'T UNCOMMENT THIS your eyes will burn
	 but it is a good test of programmatically altering just one style
	 chimara_glk_set_css_from_string(CHIMARA_GLK(glk),
	 "buffer.normal { font-family: 'Comic Sans MS'; }");*/

	GtkUIManager *uimanager = new_ui_manager("player.menus");
	gtk_ui_manager_insert_action_group(uimanager, actiongroup, 0);
	gtk_ui_manager_insert_action_group(uimanager, chimara_app_get_action_group(theapp), 1);
	GtkWidget *menubar = gtk_ui_manager_get_widget(uimanager, "/player_menu");
	self->toolbar = gtk_ui_manager_get_widget(uimanager, "/player_toolbar");
	gtk_widget_set_no_show_all(self->toolbar, TRUE);
	if(gtk_toggle_action_get_active(toolbar_action))
		gtk_widget_show(self->toolbar);
	else
		gtk_widget_hide(self->toolbar);
	
	/* Connect the accelerators */
	GtkAccelGroup *accels = gtk_ui_manager_get_accel_group(uimanager);
	gtk_window_add_accel_group(GTK_WINDOW(self), accels);

	GtkBox *vbox = GTK_BOX(load_object(builder, "player-vbox"));
	gtk_box_pack_end(vbox, self->glk, TRUE, TRUE, 0);
	g_object_ref(self->glk); /* add an extra reference to keep it alive while
							  the Glk program shuts down */
	gtk_box_pack_start(vbox, menubar, FALSE, FALSE, 0);
	gtk_box_pack_start(vbox, self->toolbar, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(vbox));
	
	gtk_builder_connect_signals(builder, self);
	g_signal_connect(self->glk, "notify::program-name", G_CALLBACK(change_window_title), self);
	g_signal_connect(self->glk, "notify::story-name", G_CALLBACK(change_window_title), self);

	g_object_unref(builder);
	g_object_unref(uimanager);
}

/* PUBLIC FUNCTIONS */

GtkWidget *
chimara_player_new(void)
{
    return GTK_WIDGET(g_object_new(CHIMARA_TYPE_PLAYER,
		"type", GTK_WINDOW_TOPLEVEL,
		NULL));
}

/* GLADE CALLBACKS */

#if 0
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
		gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_OPEN, GTK_RESPONSE_OK);
		gint response = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		
		if(response != GTK_RESPONSE_OK)
			return FALSE;

		chimara_glk_stop(glk);
		chimara_glk_wait(glk);
	}
	return TRUE;
}
#endif

void
on_stop_activate(GtkAction *action, ChimaraPlayer *player)
{
	chimara_glk_stop(CHIMARA_GLK(player->glk));
}

void
on_copy_activate(GtkAction *action, ChimaraPlayer *player)
{
	GtkWidget *focus = gtk_window_get_focus(GTK_WINDOW(player));
	/* Call "copy clipboard" on any widget that defines it */
	if(GTK_IS_LABEL(focus) || GTK_IS_ENTRY(focus) || GTK_IS_TEXT_VIEW(focus))
		g_signal_emit_by_name(focus, "copy-clipboard");
}

void
on_paste_activate(GtkAction *action, ChimaraPlayer *player)
{
	GtkWidget *focus = gtk_window_get_focus(GTK_WINDOW(player));
	/* Call "paste clipboard" on any widget that defines it */
	if(GTK_IS_ENTRY(focus) || GTK_IS_TEXT_VIEW(focus))
		g_signal_emit_by_name(focus, "paste-clipboard");
}

void
on_toolbar_toggled(GtkToggleAction *action, ChimaraPlayer *player)
{
	if(gtk_toggle_action_get_active(action))
		gtk_widget_show(player->toolbar);
	else
		gtk_widget_hide(player->toolbar);
}

void
on_undo_activate(GtkAction *action, ChimaraPlayer *player)
{
	chimara_glk_feed_line_input(CHIMARA_GLK(player->glk), "undo");
}

void 
on_save_activate(GtkAction *action, ChimaraPlayer *player)
{
	chimara_glk_feed_line_input(CHIMARA_GLK(player->glk), "save");
}

void 
on_restore_activate(GtkAction *action, ChimaraPlayer *player)
{
	chimara_glk_feed_line_input(CHIMARA_GLK(player->glk), "restore");
}

void 
on_restart_activate(GtkAction *action, ChimaraPlayer *player)
{
	chimara_glk_feed_line_input(CHIMARA_GLK(player->glk), "restart");
}

void 
on_quit_activate(GtkAction *action, ChimaraPlayer *player)
{
	chimara_glk_feed_line_input(CHIMARA_GLK(player->glk), "quit");
}

