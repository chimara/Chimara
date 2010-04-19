/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * iliad.c
 * Copyright (C) Philip en Marijn 2008 <>
 * 
 * iliad.c is free software copyrighted by Philip en Marijn.
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

/* Iliad includes */
#include <liberdm/erdm.h>
#include <liberipc/eripcviewer.h>
#include <liberipc/eripctoolbar.h>
#include <liberipc/eripcbusyd.h>

/* Global pointers to widgets */
GtkWidget *window = NULL;
GtkWidget *glk = NULL;

static erClientChannel_t erbusyChannel;
static erClientChannel_t ertoolbarChannel;

static void
on_started(ChimaraGlk *glk)
{
    g_printerr("Started!\n");
}

static void
on_stopped(ChimaraGlk *glk)
{
    g_printerr("Stopped!\n");
}

static void
on_restore()
{
	chimara_glk_feed_line_input( CHIMARA_GLK(glk), "restore" );
}

static void
on_save()
{
	chimara_glk_feed_line_input( CHIMARA_GLK(glk), "save" );
}

static void
create_window(void)
{
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	glk = chimara_if_new();
	//chimara_if_set_preferred_interpreter( CHIMARA_IF(glk), CHIMARA_IF_FORMAT_Z8, CHIMARA_IF_INTERPRETER_NITFOL);

	gtk_widget_set_size_request(window, 800, 800);
	g_object_set(glk, 
		"border-width", 6, 
		"spacing", 6,
		"ignore-errors", TRUE,
		NULL);
	chimara_glk_set_default_font_string(CHIMARA_GLK(glk), "Serif 12");
	chimara_glk_set_monospace_font_string(CHIMARA_GLK(glk), "Monospace 12");
	g_signal_connect(glk, "started", G_CALLBACK(on_started), NULL);
	g_signal_connect(glk, "stopped", G_CALLBACK(on_stopped), NULL);
	
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
	GtkWidget *toolbar = gtk_toolbar_new();

	GtkToolItem *restore_button = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
	g_signal_connect(restore_button, "clicked", G_CALLBACK(on_restore), NULL);
	gtk_toolbar_insert( GTK_TOOLBAR(toolbar), restore_button, 0 );

	GtkToolItem *save_button = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
	g_signal_connect(save_button, "clicked", G_CALLBACK(on_save), NULL);
	gtk_toolbar_insert( GTK_TOOLBAR(toolbar), save_button, 0 );

	GtkToolItem *quit_button = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
	g_signal_connect(quit_button, "clicked", G_CALLBACK(gtk_main_quit), NULL);
	gtk_toolbar_insert( GTK_TOOLBAR(toolbar), quit_button, 0 );

	GtkWidget *spacer = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_size_request(spacer, -1, 250);

	gtk_box_pack_start( GTK_BOX(vbox), toolbar, FALSE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(vbox), glk, TRUE, TRUE, 0 );
	gtk_box_pack_end( GTK_BOX(vbox), spacer, FALSE, FALSE, 0 );

	gtk_container_add( GTK_CONTAINER(window), vbox );
}

static void
iliad_init_toolbar()
{
	erIpcStartClient(ER_TOOLBAR_CHANNEL, &ertoolbarChannel);
	tbSelectIconSet(ertoolbarChannel, ER_PDF_VIEWER_UA_ID);
	tbClearIconSet(ertoolbarChannel, ER_PDF_VIEWER_UA_ID);

	// Turn off trashcan
	tbAppendPlatformIcon(  ertoolbarChannel, ER_PDF_VIEWER_UA_ID, iconID_trashcan, -1);
	tbSetStatePlatformIcon(ertoolbarChannel, ER_PDF_VIEWER_UA_ID, iconID_trashcan, iconState_grey );

	// Enable then pop up keyboard
	tbAppendPlatformIcon(  ertoolbarChannel, ER_PDF_VIEWER_UA_ID, iconID_keyboard, -1);
	tbSetStatePlatformIcon(ertoolbarChannel, ER_PDF_VIEWER_UA_ID, iconID_keyboard, iconState_selected);
}

static void
iliad_clear_toolbar()
{
	// Turn on trashcan
	tbSetStatePlatformIcon(ertoolbarChannel, ER_PDF_VIEWER_UA_ID, iconID_trashcan, iconState_normal );

	// Disable the keyboard
	tbSetStatePlatformIcon(ertoolbarChannel, ER_PDF_VIEWER_UA_ID, iconID_keyboard, iconState_normal);
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

	if(argc < 2) {
		g_printerr("Must provide a game file\n");
		return 1;
	}
	
 	if( !chimara_if_run_game(CHIMARA_IF(glk), argv[1], &error) ) {
   		g_printerr("Error starting Glk library: %s\n", error->message);
		return 1;
	}
	//chimara_glk_run( CHIMARA_GLK(glk), ".libs/multiwin.so", argc, argv, NULL);
	
	iliad_init_toolbar();

  	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

	chimara_glk_stop(CHIMARA_GLK(glk));
	chimara_glk_wait(CHIMARA_GLK(glk));

	iliad_clear_toolbar();

	return 0;
}
