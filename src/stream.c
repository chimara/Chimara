#include "stream.h"
#include <string.h>

/* Global current stream */
static strid_t current_stream = NULL;
/* List of streams currently in existence */
static GList *stream_list = NULL;

/**
 * glk_stream_iterate:
 * @str: A stream, or #NULL.
 * @rockptr: Return location for the next window's rock, or #NULL.
 *
 * Iterates over the list of streams; if @str is #NULL, it returns the first
 * stream, otherwise the next stream after @str. If there are no more, it
 * returns #NULL. The stream's rock is stored in @rockptr. If you don't want
 * the rocks to be returned, you may set @rockptr to #NULL.
 *
 * The order in which streams are returned is arbitrary. The order may change
 * every time you create or destroy a stream, invalidating the iteration.
 *
 * Returns: the next stream, or #NULL if there are no more.
 */
strid_t
glk_stream_iterate(strid_t str, glui32 *rockptr)
{
	GList *retnode;
	
	if(str == NULL)
		retnode = stream_list;
	else
		retnode = str->stream_list->next;
	strid_t retval = retnode? (strid_t)retnode->data : NULL;
		
	/* Store the stream's rock in rockptr */
	if(retval && rockptr)
		*rockptr = glk_stream_get_rock(retval);
		
	return retval;
}

/**
 * glk_stream_get_rock:
 * @str: A stream.
 * 
 * Returns the stream @str's rock value.
 *
 * Returns: A rock value.
 */
glui32
glk_stream_get_rock(strid_t str)
{
	g_return_val_if_fail(str != NULL, 0);
	return str->rock;
}

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

/* Internal function: change illegal (control) characters in a string to a
placeholder character. Must free returned string afterwards. */
static gchar *
remove_latin1_control_characters(gchar *s)
{
	gchar *retval = g_strdup(s);
	unsigned char *ptr;
	for(ptr = (unsigned char *)retval; *ptr != '\0'; ptr++)
		if( (*ptr < 32 && *ptr != 10) || (*ptr >= 127 && *ptr <= 159) )
			*ptr = '?';
			/* Our placeholder character is '?'; other options are possible,
			like printing "0x7F" or something */
	return retval;
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
	/* Illegal to print to the current stream if it is NULL */
	g_return_if_fail(current_stream != NULL);
	
	GError *error = NULL;
	gchar *canonical, *utf8;

	switch(current_stream->stream_type)
	{
		case STREAM_TYPE_WINDOW:
			canonical = remove_latin1_control_characters(s);
			utf8 = g_convert(canonical, -1, "UTF-8", "ISO-8859-1", NULL, NULL, 
			                 &error);
			g_free(canonical);
			
			if(utf8 == NULL)
			{
				g_warning("glk_put_string: "
					"Error during latin1->utf8 conversion: %s", 
					error->message);
				g_error_free(error);
				return;
			}

			/* Each window type has a different way of printing to it */
			switch(current_stream->window->window_type)
			{
				/* Printing to a these windows' streams does nothing */
				case wintype_Blank:
				case wintype_Pair:
				case wintype_Graphics:
					current_stream->write_count += strlen(s);
					break;
				/* Text buffer window */	
				case wintype_TextBuffer:
				{
					GtkTextBuffer *buffer = gtk_text_view_get_buffer( 
						GTK_TEXT_VIEW(current_stream->window->widget) );

					GtkTextIter iter;
					gtk_text_buffer_get_end_iter(buffer, &iter);

					gtk_text_buffer_insert(buffer, &iter, utf8, -1);
				}
					current_stream->write_count += strlen(s);
					break;
				default:
					g_warning("glk_put_string: "
						"Writing to this kind of window unsupported.");
			}
			
			g_free(utf8);
			break;
		default:
			g_warning("glk_put_string: "
				"Writing to this kind of stream unsupported.");	
	}
}
