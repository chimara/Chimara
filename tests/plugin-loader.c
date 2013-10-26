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
 * plugin-loader.c IS PROVIDED BY Philip en Marijn ``AS IS'' AND ANY EXPRESS
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

#include <config.h>
#include <libchimara/chimara-glk.h>

/* Global pointers to widgets */
GtkWidget *window = NULL;
GtkWidget *glk = NULL;

static gboolean
quit()
{
    gtk_main_quit();
    return TRUE;
}

static void
create_window(void)
{
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(window, 400, 400);
    g_signal_connect(window, "delete-event", G_CALLBACK(quit), NULL);
	glk = chimara_glk_new();
	g_object_ref(glk);
	gtk_container_add(GTK_CONTAINER(window), glk);
}

static gchar *
resource_load(ChimaraResourceType usage, guint32 resnum)
{
	char *resstr;
	if(usage == CHIMARA_RESOURCE_IMAGE)
		resstr = "PIC";
	else if(usage == CHIMARA_RESOURCE_SOUND)
		resstr = "SND";
	else
		resstr = "FCK";
	return g_strdup_printf("%s%d", resstr, resnum);
}

static GFile *
libname_from_la_file(char *la_filename)
{
	GFile *la_file = g_file_new_for_commandline_arg(la_filename);
	GFile *parentdir = g_file_get_parent(la_file);
	GFile *objdir = g_file_get_child(parentdir, LT_OBJDIR);
	g_object_unref(parentdir);

	GFileInputStream *istream = g_file_read(la_file, NULL, NULL);
	if(istream == NULL)
		return NULL;
	GDataInputStream *stream = g_data_input_stream_new( G_INPUT_STREAM(istream) );

	char *line;
	char *dlname = NULL;
	while( (line = g_data_input_stream_read_line(stream, NULL, NULL, NULL)) ) {
		if( g_str_has_prefix(line, "dlname=") ) {
			dlname = g_strdup( line + strlen("dlname='") );
			*(strrchr(dlname, '\'')) = '\0';
			g_free(line);
			break;
		}
		g_free(line);
	}
	if(dlname == NULL)
		return NULL;

	g_input_stream_close(G_INPUT_STREAM (stream), NULL, NULL);

	GFile *libfile = g_file_get_child(objdir, dlname);
	g_free(dlname);

	g_object_unref(la_file);
	g_object_unref(objdir);
	return libfile;
}

int
main(int argc, char *argv[])
{
	GError *error = NULL;

	gdk_threads_init();
	gtk_init(&argc, &argv);

	create_window();
	gtk_widget_show_all(window);

	if(argc < 2)
		g_error("Must provide a plugin\n");

    GFile *plugin_file;
    if( g_str_has_suffix(argv[1], ".la") )
        plugin_file = libname_from_la_file(argv[1]);
    else
        plugin_file = g_file_new_for_commandline_arg(argv[1]);

	chimara_glk_set_resource_load_callback(CHIMARA_GLK(glk), (ChimaraResourceLoadFunc)resource_load, NULL, NULL);

    if( !chimara_glk_run_file(CHIMARA_GLK(glk), plugin_file,
        argc - 1, argv + 1, &error) )
   		g_error("Error starting Glk library: %s\n", error->message);
    g_object_unref(plugin_file);

    gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

	chimara_glk_stop(CHIMARA_GLK(glk));
	chimara_glk_wait(CHIMARA_GLK(glk));
	g_object_unref(glk);

	return 0;
}
