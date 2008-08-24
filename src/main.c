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
#include <gtk/gtk.h>

#include "callbacks.h"
#include "error.h"
#include "event.h"
#include "abort.h"
#include "glk.h"

/*
 * Standard gettext macros.
 */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

/* The global builder object to be used to request handles to widgets */
GtkBuilder *builder = NULL;
	
static GtkWidget*
create_window(void)
{
	GtkWidget *window;

	if( (window = GTK_WIDGET(gtk_builder_get_object(builder, "gargoyle-gtk"))) == NULL ) {
		error_dialog(NULL, NULL, "Error while getting main window object");
		return NULL;
	}

	gtk_builder_connect_signals(builder, NULL);
	
	return window;
}

/**
 * glk_enter:
 *
 * Is called to create a new thread in which glk_main() runs.
 */
static gpointer
glk_enter(gpointer data)
{
	glk_main();
	return NULL;
}


int
main(int argc, char *argv[])
{
	GError *error = NULL;
 	GtkWidget *window;
	GThread *glk_thread;

#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
#endif

	if( !g_thread_supported() )
		g_thread_init(NULL);

	gdk_threads_init();

	gtk_set_locale();
	gtk_init(&argc, &argv);

   	builder = gtk_builder_new();
	if( !gtk_builder_add_from_file(builder, "gargoyle-gtk.ui", &error) ) {
		error_dialog(NULL, error, "Error while building interface: ");	
		return 1;
	}

	window = create_window();
	gtk_widget_show(window);

	events_init();
	interrupt_init();

	/* In een aparte thread of proces */
	if( (glk_thread = g_thread_create(glk_enter, NULL, TRUE, &error)) == NULL ) {
		error_dialog(NULL, error, "Error while creating glk thread: ");	
		g_object_unref( G_OBJECT(builder) );
		return 1;
	}

	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

	signal_abort();
	g_thread_join(glk_thread);

	g_object_unref( G_OBJECT(builder) );

	return 0;
}
