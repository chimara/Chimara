#include "stream.h"
#include "fileref.h"
#include <glib.h>
#include <glib/gstdio.h>

#include "chimara-glk-private.h"
extern ChimaraGlkPrivate *glk_data;

/* Internal function: create a window stream to go with window. */
strid_t
window_stream_new(winid_t window)
{
	/* Create stream and connect it to window */
	strid_t str = g_new0(struct glk_stream_struct, 1);
	str->file_mode = filemode_Write;
	str->type = STREAM_TYPE_WINDOW;
	str->window = window;
	
	/* Add it to the global stream list */
	glk_data->stream_list = g_list_prepend(glk_data->stream_list, str);
	str->stream_list = glk_data->stream_list;

	return str;
}

/**
 * glk_stream_iterate:
 * @str: A stream, or %NULL.
 * @rockptr: Return location for the next window's rock, or %NULL.
 *
 * Iterates through all the existing streams. See <link
 * linkend="chimara-Iterating-Through-Opaque-Objects">Iterating Through Opaque
 * Objects</link>.
 *
 * Returns: the next stream, or %NULL if there are no more.
 */
strid_t
glk_stream_iterate(strid_t str, glui32 *rockptr)
{
	GList *retnode;
	
	if(str == NULL)
		retnode = glk_data->stream_list;
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
 * Retrieves the stream @str's rock value. See <link 
 * linkend="chimara-Rocks">Rocks</link>.
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
 * @str: An output stream, or %NULL.
 *
 * Sets the current stream to @str, which must be an output stream. You may set
 * the current stream to %NULL, which means the current stream is not set to
 * anything. 
 */
void
glk_stream_set_current(strid_t str)
{
	if(str != NULL && str->file_mode == filemode_Read)
	{
		g_warning("%s: Cannot set current stream to non output stream", __func__);
		return;
	}

	glk_data->current_stream = str;
}

/**
 * glk_stream_get_current:
 * 
 * Returns the current stream, or %NULL if there is none.
 *
 * Returns: A stream, or %NULL.
 */
strid_t
glk_stream_get_current()
{
	return glk_data->current_stream;
}

/**
 * glk_put_char:
 * @ch: A character in Latin-1 encoding.
 *
 * Prints one character to the current stream. As with all basic functions, the
 * character is assumed to be in the Latin-1 character encoding. See <link
 * linkend="chimara-Character-Encoding">Character Encoding</link>.
 */
void
glk_put_char(unsigned char ch)
{
	g_return_if_fail(glk_data->current_stream != NULL);
	glk_put_char_stream(glk_data->current_stream, ch);
}

/**
 * glk_put_char_uni:
 * @ch: A Unicode code point.
 *
 * Prints one character to the current stream. The character is assumed to be a
 * Unicode code point. See <link linkend="chimara-Character-Encoding">Character
 * Encoding</link>.
 */
void
glk_put_char_uni(glui32 ch)
{
	g_return_if_fail(glk_data->current_stream != NULL);
	glk_put_char_stream_uni(glk_data->current_stream, ch);
}

/**
 * glk_put_string:
 * @s: A null-terminated string in Latin-1 encoding.
 *
 * Prints a null-terminated string to the current stream. It is exactly
 * equivalent to
 * <informalexample><programlisting>
 * for (ptr = @s; *ptr; ptr++)
 * 	#glk_put_char(*ptr);
 * </programlisting></informalexample>
 * However, it may be more efficient.
 */
void
glk_put_string(char *s)
{
	g_return_if_fail(glk_data->current_stream != NULL);
	glk_put_string_stream(glk_data->current_stream, s);
}

/**
 * glk_put_string_uni:
 * @s: A zero-terminated string of Unicode code points.
 * 
 * Prints a string of Unicode characters to the current stream. It is equivalent
 * to a series of glk_put_char_uni() calls. A string ends on a #glui32 whose
 * value is 0.
 */
void
glk_put_string_uni(glui32 *s)
{
	g_return_if_fail(glk_data->current_stream != NULL);
	glk_put_string_stream_uni(glk_data->current_stream, s);
}

/**
 * glk_put_buffer:
 * @buf: An array of characters in Latin-1 encoding.
 * @len: Length of @buf.
 *
 * Prints a block of characters to the current stream. It is exactly equivalent
 * to:
 * <informalexample><programlisting>
 * for (i = 0; i < @len; i++)
 * 	#glk_put_char(@buf[i]);
 * </programlisting></informalexample>
 * However, it may be more efficient.
 */
void
glk_put_buffer(char *buf, glui32 len)
{
	g_return_if_fail(glk_data->current_stream != NULL);
	glk_put_buffer_stream(glk_data->current_stream, buf, len);
}

/**
 * glk_put_buffer_uni:
 * @buf: An array of Unicode code points.
 * @len: Length of @buf.
 *
 * Prints a block of Unicode characters to the current stream. It is equivalent
 * to a series of glk_put_char_uni() calls.
 */
void
glk_put_buffer_uni(glui32 *buf, glui32 len)
{
	g_return_if_fail(glk_data->current_stream != NULL);
	glk_put_buffer_stream_uni(glk_data->current_stream, buf, len);
}

/**
 * glk_stream_open_memory:
 * @buf: An allocated buffer, or %NULL.
 * @buflen: Length of @buf.
 * @fmode: Mode in which the buffer will be opened. Must be one of 
 * #filemode_Read, #filemode_Write, or #filemode_ReadWrite.
 * @rock: The new stream's rock value.
 *
 * Opens a stream which reads from or writes to a space in memory. @buf points
 * to the buffer where output will be read from or written to. @buflen is the
 * length of the buffer.
 *
 * Unicode values (characters greater than 255) cannot be written to the buffer.
 * If you try, they will be stored as 0x3F (<code>"?"</code>) characters.
 *
 * Returns: the new stream, or %NULL on error.
 */
strid_t
glk_stream_open_memory(char *buf, glui32 buflen, glui32 fmode, glui32 rock)
{
	g_return_val_if_fail(fmode != filemode_WriteAppend, NULL);
	
	strid_t str = g_new0(struct glk_stream_struct, 1);
	str->rock = rock;
	str->file_mode = fmode;
	str->type = STREAM_TYPE_MEMORY;
	str->buffer = buf;
	str->mark = 0;
	str->buflen = buflen;
	str->unicode = FALSE;

	/* Add it to the global stream list */
	glk_data->stream_list = g_list_prepend(glk_data->stream_list, str);
	str->stream_list = glk_data->stream_list;

	return str;
}

/**
 * glk_stream_open_memory_uni:
 * @buf: An allocated buffer, or %NULL.
 * @buflen: Length of @buf.
 * @fmode: Mode in which the buffer will be opened. Must be one of 
 * #filemode_Read, #filemode_Write, or #filemode_ReadWrite.
 * @rock: The new stream's rock value.
 *
 * Works just like glk_stream_open_memory(), except that the buffer is an array
 * of 32-bit words, instead of bytes. This allows you to write and read any
 * Unicode character. The @buflen is the number of words, not the number of
 * bytes.
 * 
 * Returns: the new stream, or %NULL on error.
 */
strid_t
glk_stream_open_memory_uni(glui32 *buf, glui32 buflen, glui32 fmode, glui32 rock)
{
	g_return_val_if_fail(fmode != filemode_WriteAppend, NULL);
	
	strid_t str = g_new0(struct glk_stream_struct, 1);
	str->rock = rock;
	str->file_mode = fmode;
	str->type = STREAM_TYPE_MEMORY;
	str->ubuffer = buf;
	str->mark = 0;
	str->buflen = buflen;
	str->unicode = TRUE;

	/* Add it to the global stream list */
	glk_data->stream_list = g_list_prepend(glk_data->stream_list, str);
	str->stream_list = glk_data->stream_list;

	return str;
}

/* Internal function: create a stream using the given parameters. */
static strid_t
file_stream_new(frefid_t fileref, glui32 fmode, glui32 rock, gboolean unicode)
{
	g_return_val_if_fail(fileref != NULL, NULL);
	
	gchar *modestr;
	/* Binary mode is 0x000, text mode 0x100 */
	gboolean binary = !(fileref->usage & fileusage_TextMode);
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
			if( g_file_test(fileref->filename, G_FILE_TEST_EXISTS) ) {
				modestr = g_strdup(binary? "r+b" : "r+");
			} else {
				modestr = g_strdup(binary? "w+b" : "w+");
			}
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
		gdk_threads_enter();

		GtkWidget *dialog = gtk_message_dialog_new(NULL, 0,
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
			"File %s already exists. Overwrite?", fileref->filename);
		gint response = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		gdk_threads_leave();

		if(response != GTK_RESPONSE_YES) {
			fclose(fp);
			return NULL;
		}
	}
	
	strid_t str = g_new0(struct glk_stream_struct, 1);
	str->rock = rock;
	str->file_mode = fmode;
	str->type = STREAM_TYPE_FILE;
	str->file_pointer = fp;
	str->binary = binary;
	str->unicode = unicode;
	str->filename = g_filename_to_utf8(fileref->filename, -1, NULL, NULL, NULL);
	if(str->filename == NULL)
		str->filename = g_strdup("Unknown file name"); /* fail silently */
	/* Add it to the global stream list */
	glk_data->stream_list = g_list_prepend(glk_data->stream_list, str);
	str->stream_list = glk_data->stream_list;

	return str;
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
 * When writing in binary mode, Unicode values (characters greater than 255)
 * cannot be written to the file. If you try, they will be stored as 0x3F
 * (<code>"?"</code>) characters. In text mode, Unicode values may be stored
 * exactly, approximated, or abbreviated, depending on what the platform's text
 * files support.
 *
 * Returns: A new stream, or %NULL if the file operation failed.
 */
strid_t
glk_stream_open_file(frefid_t fileref, glui32 fmode, glui32 rock)
{
	return file_stream_new(fileref, fmode, rock, FALSE);
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
 * In text mode, the file is written and read in a platform-dependent way, which
 * may or may not handle all Unicode characters. A text-mode file created with
 * glk_stream_open_file_uni() may have the same format as a text-mode file
 * created with glk_stream_open_file(); or it may use a more Unicode-friendly
 * format.
 *
 * Returns: A new stream, or %NULL if the file operation failed.
 */
strid_t
glk_stream_open_file_uni(frefid_t fileref, glui32 fmode, glui32 rock)
{
	return file_stream_new(fileref, fmode, rock, TRUE);
}

/**
 * glk_stream_close:
 * @str: Stream to close.
 * @result: Pointer to a #stream_result_t, or %NULL.
 *
 * Closes the stream @str. The @result argument points to a structure which is
 * filled in with the final character counts of the stream. If you do not care
 * about these, you may pass %NULL as the @result argument.
 *
 * If @str is the current output stream, the current output stream is set to
 * %NULL.
 *
 * You cannot close window streams; use glk_window_close() instead. See <link
 * linkend="chimara-Window-Opening-Closing-and-Constraints">Window Opening,
 * Closing, and Constraints</link>.
 */
void 
glk_stream_close(strid_t str, stream_result_t *result)
{
	g_return_if_fail(str != NULL);
	
	/* Free resources associated with one specific type of stream */
	switch(str->type)
	{
		case STREAM_TYPE_WINDOW:
			g_warning("%s: Attempted to close a window stream. Use glk_window_"
				"close() instead.", __func__);
			return;
			
		case STREAM_TYPE_MEMORY:
			/* Do nothing */
			break;
			
		case STREAM_TYPE_FILE:
			if(fclose(str->file_pointer) != 0)
				g_warning("%s: Failed to close file '%s'.", __func__, 
					str->filename);
			g_free(str->filename);
			break;
		default:
			g_warning("%s: Closing this type of stream not supported.", 
				__func__);
			return;
	}

	stream_close_common(str, result);
}

/* Internal function: Stuff to do upon closing any type of stream. */
void
stream_close_common(strid_t str, stream_result_t *result)
{
	/* Remove the stream from the global stream list */
	glk_data->stream_list = g_list_delete_link(glk_data->stream_list, str->stream_list);
	
	/* If it was the current output stream, set that to NULL */
	if(glk_data->current_stream == str)
		glk_data->current_stream = NULL;
		
	/* If it was one or more windows' echo streams, set those to NULL */
	winid_t win;
	for(win = glk_window_iterate(NULL, NULL); win; 
		win = glk_window_iterate(win, NULL))
		if(win->echo_stream == str)
			win->echo_stream = NULL;
			
	/* Return the character counts */
	if(result) 
	{
		result->readcount = str->read_count;
		result->writecount = str->write_count;
	}
	g_free(str);
}
