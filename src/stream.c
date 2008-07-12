#include "stream.h"

/* Global current stream */
static strid_t current_stream = NULL;
static GList *stream_list = NULL;

strid_t
window_stream_new(winid_t window)
{

	strid_t s = g_new0(struct glk_stream_struct, 1);
	s->file_mode = filemode_Write;
	s->stream_type = STREAM_TYPE_WINDOW;
	s->window = window;

	stream_list = g_list_prepend(stream_list, s);
	s->stream_list = stream_list;

	return s;
}

void
glk_stream_set_current(strid_t stream)
{
	if(stream->file_mode != filemode_Write)
	{
		g_warning("glk_stream_set_current: Cannot set current stream to non output stream");
		return;
	}

	current_stream = stream;
}

void
glk_put_string(char *s)
{
	GError *error = NULL;
	gchar *utf8;

	switch(current_stream->stream_type)
	{
		case STREAM_TYPE_WINDOW:
			utf8 = g_convert(s, -1, "UTF-8", "ISO-8859-1", NULL, NULL, &error);

			if(utf8 == NULL)
			{
				g_warning("glk_put_string: Error during latin1->utf8 conversion: %s", error->message);
				g_error_free(error);
			}

			GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(current_stream->window->widget) );

			GtkTextIter iter;
			gtk_text_buffer_get_end_iter(buffer, &iter);

			gtk_text_buffer_insert(buffer, &iter, utf8, -1);

			g_free(utf8);
			break;
		default:
			g_warning("glk_put_string: Writing to this kind of stream unsupported.");	
	}
}
