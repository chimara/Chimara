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

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libchimara/chimara-glk.h>
#include <libchimara/chimara-if.h>
#include "error.h"

void 
on_open_activate(GtkAction *action, ChimaraGlk *glk) 
{
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
			return;

		chimara_glk_stop(glk);
		chimara_glk_wait(glk);
	}

	GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Open Game"),
	    window,
	    GTK_FILE_CHOOSER_ACTION_OPEN,
	    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
	    NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		GError *error = NULL;
		gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		if(!chimara_if_run_game(CHIMARA_IF(glk), filename, &error))
			error_dialog(window, error, _("Could not open game file."));
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
}

void
on_recent_item_activated(GtkRecentChooser *chooser, ChimaraGlk *glk)
{

}

void
on_stop_activate(GtkAction *action, ChimaraGlk *glk)
{
	chimara_glk_stop(glk);
}

void 
on_quit_chimara_activate(GtkAction *action, ChimaraGlk *glk) 
{
	gtk_main_quit();
}

void
on_cut_activate(GtkAction *action, ChimaraGlk *glk)
{

}

void
on_copy_activate(GtkAction *action, ChimaraGlk *glk)
{

}

void
on_paste_activate(GtkAction *action, ChimaraGlk *glk)
{

}

void
on_preferences_activate(GtkAction *action, ChimaraGlk *glk)
{

}

void
on_undo_activate(GtkAction *action, ChimaraGlk *glk)
{
	chimara_glk_feed_line_input(glk, "undo");
}

void 
on_save_activate(GtkAction *action, ChimaraGlk *glk) 
{
	chimara_glk_feed_line_input(glk, "save");
}

void 
on_restore_activate(GtkAction *action, ChimaraGlk *glk) 
{
	chimara_glk_feed_line_input(glk, "restore");
}

void 
on_restart_activate(GtkAction *action, ChimaraGlk *glk) 
{
	chimara_glk_feed_line_input(glk, "restart");
}

void 
on_quit_activate(GtkAction *action, ChimaraGlk *glk) 
{
	chimara_glk_feed_line_input(glk, "quit");
}

void
on_about_activate(GtkAction *action, ChimaraGlk *glk)
{

}

gboolean 
on_window_delete_event(GtkWidget *widget, GdkEvent *event, ChimaraGlk *glk) 
{
	gtk_main_quit();
	return TRUE;
}
