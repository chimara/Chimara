#include "config.h"

#include <string.h>

#include <gio/gio.h>

GFile *
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
