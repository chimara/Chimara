#include "stream.h"

/* Global current stream */
static strid_t current_stream = NULL;
/* List of streams currently in existence */
static GList *stream_list = NULL;

/* Internal function: create a window stream to go with window. */
strid_t
window_stream_new(winid_t window)
{
	/* Create stream and connect it to window */
	strid_t s = g_new0(struct glk_stream_struct, 1);
	s->file_mode = filemode_Write;
	s->stream_type = STREAM_TYPE_WINDOW;
	s->window = window;
	/* Add it to the global stream list */
	stream_list = g_list_prepend(stream_list, s);
	s->stream_list = stream_list;

	return s;
}

/**
 * glk_stream_set_current:
 * @str: An output stream, or NULL.
 *
 * Sets the current stream to @str, or to nothing if @str is #NULL.
 */
void
glk_stream_set_current(strid_t str)
{
	if(str != NULL && str->file_mode != filemode_Write)
	{
		g_warning("glk_stream_set_current: "
			"Cannot set current stream to non output stream");
		return;
	}

	current_stream = str;
}

/**
 * glk_put_string:
 * @s: A null-terminated string in Latin-1 encoding.
 *
 * Prints @s to the current stream.
 */
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
				g_warning("glk_put_string: "
					"Error during latin1->utf8 conversion: %s", 
					error->message);
				g_error_free(error);
				return;
			}

			GtkTextBuffer *buffer = gtk_text_view_get_buffer( 
				GTK_TEXT_VIEW(current_stream->window->widget) );

			GtkTextIter iter;
			gtk_text_buffer_get_end_iter(buffer, &iter);

			gtk_text_buffer_insert(buffer, &iter, utf8, -1);

			g_free(utf8);
			break;
		default:
			g_warning("glk_put_string: "
				"Writing to this kind of stream unsupported.");	
	}
}
