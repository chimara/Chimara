#include "stream.h"
#include "fileref.h"
#include <string.h>
#include <stdio.h>
#include <glib/gstdio.h>

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
 * glk_stream_get_current:
 * 
 * Returns the current stream, or #NULL if there is none.
 *
 * Returns: A stream.
 */
strid_t
glk_stream_get_current()
{
	return current_stream;
}

/**
 * glk_put_char:
 * @ch: A character in Latin-1 encoding.
 *
 * Prints one character @ch to the current stream.
 */
void
glk_put_char(unsigned char ch)
{
	/* Illegal to print to the current stream if it is NULL */
	g_return_if_fail(current_stream != NULL);
	glk_put_char_stream(current_stream, ch);
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
	glk_put_string_stream(current_stream, s);
}

/**
 * glk_put_buffer:
 * @buf: An array of characters in Latin-1 encoding.
 * @len: Length of @buf.
 *
 * Prints @buf to the current stream.
 */
void
glk_put_buffer(char *buf, glui32 len)
{
	/* Illegal to print to the current stream if it is NULL */
	g_return_if_fail(current_stream != NULL);
	glk_put_buffer_stream(current_stream, buf, len);
}

/**
 * glk_put_char_stream:
 * @str: An output stream.
 * @ch: A character in Latin-1 encoding.
 *
 * Prints one character @ch to the stream @str. It is illegal for @str to be
 * #NULL, or an input-only stream.
 */
void
glk_put_char_stream(strid_t str, unsigned char ch)
{
	g_return_if_fail(str != NULL);
	g_return_if_fail(str->file_mode != filemode_Read);
	
	/* Convert ch to a null-terminated string, call glk_put_string_stream() */
	gchar *s = g_strndup((gchar *)&ch, 1);
	glk_put_string_stream(str, s);
	g_free(s);
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

/* Internal function: convert a Latin-1 string to a UTF-8 string, replacing
Latin-1 control characters by a placeholder first. The UTF-8 string must be
freed afterwards. Returns NULL on error. */
static gchar *
convert_latin1_to_utf8(gchar *s)
{
	GError *error = NULL;
	gchar *utf8;
	gchar *canonical = remove_latin1_control_characters(s);
	utf8 = g_convert(canonical, -1, "UTF-8", "ISO-8859-1", NULL, NULL, &error);
	g_free(canonical);
	
	if(utf8 == NULL)
	{
		error_dialog(NULL, error, "Error during latin1->utf8 conversion: ");
		return NULL;
	}
	
	return utf8;
}

/* Internal function: write a UTF-8 string to a window's text buffer. */
static void
write_utf8_to_window(winid_t win, gchar *s)
{
	GtkTextBuffer *buffer = 
		gtk_text_view_get_buffer( GTK_TEXT_VIEW(win->widget) );

	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(buffer, &iter);
	gtk_text_buffer_insert(buffer, &iter, s, -1);
}

/**
 * glk_put_string_stream:
 * @str: An output stream.
 * @s: A null-terminated string in Latin-1 encoding.
 *
 * Prints @s to the stream @str. It is illegal for @str to be #NULL, or an
 * input-only stream.
 */
void
glk_put_string_stream(strid_t str, char *s)
{
	g_return_if_fail(str != NULL);
	g_return_if_fail(str->file_mode != filemode_Read);

	switch(str->stream_type)
	{
		case STREAM_TYPE_WINDOW:
			/* Each window type has a different way of printing to it */
			switch(str->window->window_type)
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
					gchar *utf8 = convert_latin1_to_utf8(s);
					write_utf8_to_window(str->window, utf8);
					g_free(utf8);
				}	
					str->write_count += strlen(s);
					break;
				default:
					g_warning("glk_put_string: "
						"Writing to this kind of window unsupported.");
			}
			
			/* Now write the same buffer to the window's echo stream */
			if(str->window->echo_stream != NULL)
				glk_put_string_stream(str->window->echo_stream, s);
			
			break;
		default:
			g_warning("glk_put_string: "
				"Writing to this kind of stream unsupported.");	
	}
}

/**
 * glk_put_buffer_stream:
 * @str: An output stream.
 * @buf: An array of characters in Latin-1 encoding.
 * @len: Length of @buf.
 *
 * Prints @buf to the stream @str. It is illegal for @str to be #NULL, or an
 * input-only stream.
 */
void
glk_put_buffer_stream(strid_t str, char *buf, glui32 len)
{
	g_return_if_fail(str != NULL);
	g_return_if_fail(str->file_mode != filemode_Read);
	
	/* Convert buf to a null-terminated string, call glk_put_string_stream() */
	gchar *s = g_strndup(buf, len);
	glk_put_string_stream(str, s);
	g_free(s);
}

/* Internal function: create a stream using the given parameters. */
static strid_t
stream_new(frefid_t fileref, glui32 fmode, glui32 rock, gboolean unicode)
{
	g_return_val_if_fail(fileref != NULL, NULL);
	
	gchar *modestr;
	gboolean binary = fileref->usage & fileusage_BinaryMode;
	switch(fmode) 
	{
		case filemode_Read:
			if(!g_file_test(fileref->filename, G_FILE_TEST_EXISTS)) {
				g_warning("glk_stream_open_file: Tried to open a file in read "
						  "mode that didn't exist!");
				return NULL;
			}
			modestr = g_strdup(binary? "rb" : "r");
			break;
		case filemode_Write:
			modestr = g_strdup(binary? "wb" : "w");
			break;
		case filemode_WriteAppend:
			modestr = g_strdup(binary? "ab" : "a");
			break;
		case filemode_ReadWrite:
			modestr = g_strdup(binary? "r+b" : "r+");
			break;
		default:
			g_warning("glk_stream_open_file: Invalid file mode");
			return NULL;
	}
	
	FILE *fp = g_fopen(fileref->filename, modestr);
	g_free(modestr);
	if(fp == NULL) {
		g_warning("glk_stream_open_file: Error opening file");
		return NULL;
	}
	
	/* If they opened a file in write mode but didn't specifically get
	permission to do so, complain if the file already exists */
	if(fileref->orig_filemode == filemode_Read && fmode != filemode_Read) {
		GtkWidget *dialog = gtk_message_dialog_new(NULL, 0,
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
			"File %s already exists. Overwrite?", fileref->filename);
		gint response = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		if(response != GTK_RESPONSE_YES) {
			fclose(fp);
			return NULL;
		}
	}
	
	strid_t s = g_new0(struct glk_stream_struct, 1);
	s->file_mode = fmode;
	s->stream_type = unicode? STREAM_TYPE_UNICODE_FILE : STREAM_TYPE_FILE;
	s->file_pointer = fp;
	s->binary = binary;
	/* Add it to the global stream list */
	stream_list = g_list_prepend(stream_list, s);
	s->stream_list = stream_list;

	return s;
}

/**
 * glk_stream_open_file:
 * @fileref: Indicates the file which will be opened.
 * @fmode: Mode in which the file will be opened. Can be any of #filemode_Read,
 * #filemode_Write, #filemode_WriteAppend, or #filemode_ReadWrite.
 * @rock: The new stream's rock value.
 *
 * Opens a stream which reads to or writes from a disk file. If @fmode is
 * #filemode_Read, the file must already exist; for the other modes, an empty
 * file is created if none exists. If @fmode is #filemode_Write, and the file
 * already exists, it is truncated down to zero length (an empty file). If
 * @fmode is #filemode_WriteAppend, the file mark is set to the end of the 
 * file.
 * 
 * The file may be written in text or binary mode; this is determined by the
 * @fileref argument. Similarly, platform-dependent attributes such as file 
 * type are determined by @fileref.
 *
 * When writing in binary mode, Unicode values (characters greater than 255)
 * cannot be written to the file. If you try, they will be stored as 0x3F ("?")
 * characters. In text mode, Unicode values are stored in UTF-8.
 *
 * Returns: A new stream, or %NULL if the file operation failed.
 */
strid_t
glk_stream_open_file(frefid_t fileref, glui32 fmode, glui32 rock)
{
	return stream_new(fileref, fmode, rock, FALSE);
}

/**
 * glk_stream_open_file_uni:
 * @fileref: Indicates the file which will be opened.
 * @fmode: Mode in which the file will be opened. Can be any of #filemode_Read,
 * #filemode_Write, #filemode_WriteAppend, or #filemode_ReadWrite.
 * @rock: The new stream's rock value.
 *
 * This works just like glk_stream_open_file(), except that in binary mode,
 * characters are written and read as four-byte (big-endian) values. This
 * allows you to write any Unicode character.
 *
 * In text mode, the file is written and read in UTF-8.
 *
 * Returns: A new stream, or %NULL if the file operation failed.
 */
strid_t
glk_stream_open_file_uni(frefid_t fileref, glui32 fmode, glui32 rock)
{
	return stream_new(fileref, fmode, rock, TRUE);
}

