#include <config.h>
#include "stream.h"
#include "fileref.h"
#include "magic.h"
#include "gi_blorb.h"
#include <errno.h>
#include <stdio.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib/gi18n-lib.h>

#include "chimara-glk-private.h"
extern GPrivate glk_data_key;

/* Internal function: create a stream with a specified rock value */
strid_t
stream_new_common(glui32 rock)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);

	strid_t str = g_slice_new0(struct glk_stream_struct);
	str->magic = MAGIC_STREAM;
	str->rock = rock;
	if(glk_data->register_obj)
		str->disprock = (*glk_data->register_obj)(str, gidisp_Class_Stream);
		
	/* Add it to the global stream list */
	glk_data->stream_list = g_list_prepend(glk_data->stream_list, str);
	str->stream_list = glk_data->stream_list;

	return str;
}

/* Internal function: Stuff to do upon closing any type of stream. Call only 
 from Glk thread. */
void
stream_close_common(strid_t str, stream_result_t *result)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);

	if(glk_data->unregister_obj)
	{
		(*glk_data->unregister_obj)(str, gidisp_Class_Stream, str->disprock);
		str->disprock.ptr = NULL;
	}
	
	/* If the stream was one or more windows' echo streams, set those to NULL */
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

	/* Remove the stream from the global stream list */
	glk_data->stream_list = g_list_delete_link(glk_data->stream_list, str->stream_list);
	
	/* If it was the current output stream, set that to NULL */
	if(glk_data->current_stream == str)
		glk_data->current_stream = NULL;
	
	str->magic = MAGIC_FREE;
	g_slice_free(struct glk_stream_struct, str);
}

/**
 * glk_stream_iterate:
 * @str: A stream, or %NULL.
 * @rockptr: Return location for the next window's rock, or %NULL.
 *
 * Iterates through all the existing streams.
 * See [Iterating Through Opaque
 * Objects][chimara-Iterating-Through-Opaque-Objects].
 *
 * Returns: the next stream, or %NULL if there are no more.
 */
strid_t
glk_stream_iterate(strid_t str, glui32 *rockptr)
{
	VALID_STREAM_OR_NULL(str, return NULL);

	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
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
 * Retrieves the stream @str's rock value.
 * See [Rocks][chimara-Rocks].
 * Window streams always have rock 0; all other streams return whatever rock you
 * created them with.
 *
 * Returns: A rock value.
 */
glui32
glk_stream_get_rock(strid_t str)
{
	VALID_STREAM(str, return 0);
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
	VALID_STREAM_OR_NULL(str, return);

	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);

	if(str != NULL && str->file_mode == filemode_Read)
	{
		ILLEGAL("Cannot set current stream to non output stream");
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
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	return glk_data->current_stream;
}

/**
 * glk_put_char:
 * @ch: A character in Latin-1 encoding.
 *
 * Prints one character to the current stream. As with all basic functions, the
 * character is assumed to be in the Latin-1 character encoding.
 * See [Character Encoding][chimara-Character-Encoding].
 */
void
glk_put_char(unsigned char ch)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	VALID_STREAM(glk_data->current_stream, return);
	glk_put_char_stream(glk_data->current_stream, ch);
}

/**
 * glk_put_char_uni:
 * @ch: A Unicode code point.
 *
 * Prints one character to the current stream. The character is assumed to be a
 * Unicode code point.
 * See [Character Encoding][chimara-Character-Encoding].
 */
void
glk_put_char_uni(glui32 ch)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	VALID_STREAM(glk_data->current_stream, return);
	glk_put_char_stream_uni(glk_data->current_stream, ch);
}

/**
 * glk_put_string:
 * @s: A null-terminated string in Latin-1 encoding.
 *
 * Prints a null-terminated string to the current stream. It is exactly
 * equivalent to
 * |[<!--language="C"-->
 * for (ptr = s; *ptr; ptr++)
 *     glk_put_char(*ptr);
 * ]|
 * However, it may be more efficient.
 */
void
glk_put_string(char *s)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	VALID_STREAM(glk_data->current_stream, return);
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
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	VALID_STREAM(glk_data->current_stream, return);
	glk_put_string_stream_uni(glk_data->current_stream, s);
}

/**
 * glk_put_buffer:
 * @buf: An array of characters in Latin-1 encoding.
 * @len: Length of @buf.
 *
 * Prints a block of characters to the current stream. It is exactly equivalent
 * to:
 * |[<!--language="C"-->
 * for (i = 0; i < len; i++)
 *     glk_put_char(buf[i]);
 * ]|
 * However, it may be more efficient.
 */
void
glk_put_buffer(char *buf, glui32 len)
{
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	VALID_STREAM(glk_data->current_stream, return);
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
	ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
	VALID_STREAM(glk_data->current_stream, return);
	glk_put_buffer_stream_uni(glk_data->current_stream, buf, len);
}

/**
 * glk_stream_open_memory:
 * @buf: An allocated buffer, or %NULL.
 * @buflen: Length of @buf.
 * @fmode: Mode in which the buffer will be opened. Must be one of 
 * %filemode_Read, %filemode_Write, or %filemode_ReadWrite.
 * @rock: The new stream's rock value.
 *
 * Opens a stream which reads from or writes to a space in memory. @buf points
 * to the buffer where output will be read from or written to. @buflen is the
 * length of the buffer.
 *
 * Unicode values (characters greater than 255) cannot be written to the buffer.
 * If you try, they will be stored as 0x3F (`"?"`) characters.
 *
 * Returns: the new stream, or %NULL on error.
 */
strid_t
glk_stream_open_memory(char *buf, glui32 buflen, glui32 fmode, glui32 rock)
{
	g_return_val_if_fail(fmode != filemode_WriteAppend, NULL);
	
	strid_t str = stream_new_common(rock);
	str->file_mode = fmode;
	str->type = STREAM_TYPE_MEMORY;
	str->mark = 0;
	str->endmark = 0;
	str->unicode = FALSE;

	if(buf && buflen) 
	{
		ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
		str->buffer = buf;
		str->buflen = buflen;
		if(glk_data->register_arr) 
			str->buffer_rock = (*glk_data->register_arr)(buf, buflen, "&+#!Cn");
	}
	
	return str;
}

/**
 * glk_stream_open_memory_uni:
 * @buf: An allocated buffer, or %NULL.
 * @buflen: Length of @buf.
 * @fmode: Mode in which the buffer will be opened. Must be one of 
 * %filemode_Read, %filemode_Write, or %filemode_ReadWrite.
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
	
	strid_t str = stream_new_common(rock);
	str->file_mode = fmode;
	str->type = STREAM_TYPE_MEMORY;
	str->mark = 0;
	str->endmark = 0;
	str->unicode = TRUE;

	if(buf && buflen) 
	{
		ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
		str->ubuffer = buf;
		str->buflen = buflen;
		if(glk_data->register_arr) 
			str->buffer_rock = (*glk_data->register_arr)(buf, buflen, "&+#!Iu");
	}
	
	return str;
}

/* Internal function: create a stream using the given parameters. */
strid_t
file_stream_new(frefid_t fileref, glui32 fmode, glui32 rock, gboolean unicode)
{
	VALID_FILEREF(fileref, return NULL);
	
	const gchar *modestr;
	/* Binary mode is 0x000, text mode 0x100 */
	gboolean binary = !(fileref->usage & fileusage_TextMode);
	switch(fmode) 
	{
		case filemode_Read:
			if(!g_file_test(fileref->filename, G_FILE_TEST_EXISTS)) {
				ILLEGAL_PARAM("Tried to open a nonexistent file, '%s', in read mode", fileref->filename);
				return NULL;
			}
			modestr = binary? "rb" : "r";
			break;
		case filemode_Write:
			modestr = binary? "wb" : "w";
			break;
		case filemode_WriteAppend:
		case filemode_ReadWrite:
		{
			/* We have to open the file first and then close it, in order to
			 both make sure it exists and be able to seek in it later */
			FILE *fp = g_fopen(fileref->filename, binary? "ab" : "a");
			if(fclose(fp) != 0) {
				IO_WARNING( "Error opening file", fileref->filename, g_strerror(errno) );
				return NULL;
			}
			modestr = binary? "r+b" : "r+";
		}
			break;
		default:
			ILLEGAL_PARAM("Invalid file mode: %u", fmode);
			return NULL;
	}
	
	FILE *fp = g_fopen(fileref->filename, modestr);
	if(fp == NULL) {
		IO_WARNING( "Error opening file", fileref->filename, g_strerror(errno) );
		return NULL;
	}
	
	/* Fast-forward to the end if we are appending */
	if(fmode == filemode_WriteAppend && fseek(fp, 0, SEEK_END) != 0) {
		IO_WARNING("Error fast-forwarding file to end", fileref->filename, g_strerror(errno));
		return NULL;
	}

	/* If they opened a file in write mode but didn't specifically get
	permission to do so, complain if the file already exists */
	if(fileref->orig_filemode == filemode_Read && fmode != filemode_Read) {
		gdk_threads_enter();

		GtkWidget *dialog = gtk_message_dialog_new(NULL, 0,
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
			"File '%s' already exists. Overwrite?", fileref->filename);
		gint response = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		gdk_threads_leave();

		if(response != GTK_RESPONSE_YES) {
			if(fclose(fp) != 0)
				IO_WARNING( "Error closing file", fileref->filename, g_strerror(errno) );
			return NULL;
		}
	}
	
	strid_t str = stream_new_common(rock);
	str->file_mode = fmode;
	str->type = STREAM_TYPE_FILE;
	str->file_pointer = fp;
	str->binary = binary;
	str->unicode = unicode;
	str->filename = g_filename_to_utf8(fileref->filename, -1, NULL, NULL, NULL);
	if(str->filename == NULL)
		str->filename = g_strdup("Unknown file name"); /* fail silently */

	return str;
}

/**
 * glk_stream_open_file:
 * @fileref: Indicates the file which will be opened.
 * @fmode: Mode in which the file will be opened. Can be any of %filemode_Read,
 * %filemode_Write, %filemode_WriteAppend, or %filemode_ReadWrite.
 * @rock: The new stream's rock value.
 *
 * Opens a stream which reads to or writes from a disk file. If @fmode is
 * %filemode_Read, the file must already exist; for the other modes, an empty
 * file is created if none exists. If @fmode is %filemode_Write, and the file
 * already exists, it is truncated down to zero length (an empty file); the
 * other modes do not truncate. If @fmode is %filemode_WriteAppend, the file
 * mark is set to the end of the file.
 *
 * <note><para>
 *   Note, again, that this doesn't match stdio's fopen() call very well. See
 *   <link linkend="filemode-WriteAppend">the file mode constants</link>.
 * </para></note>
 *
 * If the filemode requires the file to exist, but the file does not exist,
 * glk_stream_open_file() returns %NULL.
 *
 * The file may be read or written in text or binary mode; this is determined
 * by the @fileref argument. Similarly, platform-dependent attributes such as
 * file type are determined by @fileref.
 * See [File References][chimara-File-References].
 *
 * When writing in binary mode, Unicode values (characters greater than 255)
 * cannot be written to the file. If you try, they will be stored as 0x3F
 * (`"?"`) characters. In text mode, Unicode values may be stored
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
 * @fmode: Mode in which the file will be opened. Can be any of %filemode_Read,
 * %filemode_Write, %filemode_WriteAppend, or %filemode_ReadWrite.
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
 * glk_stream_open_resource:
 * @filenum: Resource chunk number to open.
 * @rock: The new stream's rock value.
 *
 * Open the given data resource for reading (only), as a normal stream.
 *
 * <note><para>
 *   Note that there is no notion of file usage &mdash; the resource does not
 *   have to be specified as “saved game” or whatever.
 * </para></note>
 *
 * If no resource chunk of the given number exists, the open function returns
 * %NULL.
 *
 * As with file streams, a binary resource stream reads the resource as bytes. A
 * text resource stream reads characters encoded as Latin-1.
 *
 * When reading from a resource stream, newlines are not remapped, even if they
 * normally would be when reading from a text file on the host OS. If you read a
 * line (glk_get_line_stream() or glk_get_line_stream_uni()), a Unix newline
 * (0x0A) terminates the line.
 *
 * Returns: the new stream, or %NULL.
 */
strid_t
glk_stream_open_resource(glui32 filenum, glui32 rock)
{
	/* Adapted from CheapGlk */
	strid_t str;
	gboolean isbinary;
	giblorb_err_t err;
	giblorb_result_t res;
	giblorb_map_t *map = giblorb_get_resource_map();
	if(map == NULL) {
		WARNING(_("Could not create resource stream, because there was no "
			"resource map."));
		return NULL; /* Not running from a blorb file */
	}

	err = giblorb_load_resource(map, giblorb_method_Memory, &res, giblorb_ID_Data, filenum);
	if(err) {
		WARNING_S(_("Could not create resource stream, because the resource "
			"could not be loaded"), giblorb_get_error_message(err));
		return 0; /* Not found, or some other error */
	}

	/* We'll use the in-memory copy of the chunk data as the basis for
	our new stream. It's important to not call chunk_unload() until
	the stream is closed (and we won't).

	This will be memory-hoggish for giant data chunks, but I don't
	expect giant data chunks at this point. A more efficient model
	would be to use the file on disk, but this requires some hacking
	into the file stream code (we'd need to open a new FILE*) and
	I don't feel like doing that. */

	if(res.chunktype == giblorb_ID_TEXT)
		isbinary = FALSE;
	else if(res.chunktype == giblorb_ID_BINA)
		isbinary = TRUE;
	else {
		WARNING(_("Could not create resource stream, because chunk was of "
			"unknown type."));
		return NULL; /* Unknown chunk type */
	}

	str = stream_new_common(rock);
	str->type = STREAM_TYPE_RESOURCE;
	str->file_mode = filemode_Read;
	str->binary = isbinary;

	if (res.data.ptr && res.length) {
		str->buffer = res.data.ptr;
		str->mark = 0;
		str->buflen = res.length;
		str->endmark = str->buflen;
	}

	return str;
}

/**
 * glk_stream_open_resource_uni:
 * @filenum: Resource chunk number to open.
 * @rock: The new stream's rock value.
 *
 * Open the given data resource for reading (only), as a Unicode stream. See
 * glk_stream_open_resource() for more information.
 *
 * As with file streams, a binary resource stream reads the resource as
 * four-byte (big-endian) words. A text resource stream reads characters encoded
 * as UTF-8.
 *
 * <note><para>
 *   Thus, the difference between text and binary resources is only important
 *   when opened as a Unicode stream.
 * </para></note>
 *
 * Returns: the new stream, or %NULL.
 */
strid_t
glk_stream_open_resource_uni(glui32 filenum, glui32 rock)
{
	/* Adapted from CheapGlk */
	/* We have been handed an array of bytes. (They're big-endian
	four-byte chunks, or perhaps a UTF-8 byte sequence, rather than
	native-endian four-byte integers). So we drop it into str->buffer,
	rather than str->ubuffer -- we'll have to do the translation in the
	get() functions. */
	strid_t str = glk_stream_open_resource(filenum, rock);
	if(str != NULL)
		str->unicode = TRUE;
	return str;
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
 * You cannot close window streams; use glk_window_close() instead.
 * See [Window Opening, Closing, and
 * Constraints][chimara-Window-Opening-Closing-and-Constraints].
 */
void 
glk_stream_close(strid_t str, stream_result_t *result)
{
	VALID_STREAM(str, return);
	
	/* Free resources associated with one specific type of stream */
	switch(str->type)
	{
		case STREAM_TYPE_WINDOW:
			ILLEGAL("Attempted to close a window stream. Use glk_window_close() instead.");
			return;
			
		case STREAM_TYPE_MEMORY:
		{
			ChimaraGlkPrivate *glk_data = g_private_get(&glk_data_key);
			if(glk_data->unregister_arr) 
			{
				if(str->unicode)
					(*glk_data->unregister_arr)(str->ubuffer, str->buflen, "&+#!Iu", str->buffer_rock);
				else
					(*glk_data->unregister_arr)(str->buffer, str->buflen, "&+#!Cn", str->buffer_rock);
            }
		}
			break;
		
		case STREAM_TYPE_FILE:
			if(fclose(str->file_pointer) != 0)
				IO_WARNING( "Failed to close file", str->filename, g_strerror(errno) );
			g_free(str->filename);
			break;

		case STREAM_TYPE_RESOURCE:
			/* Shouldn't free the chunk; someone else might be using it. It will
			be freed when the resource map is freed. */
			break;

		default:
			ILLEGAL_PARAM("Unknown stream type: %u", str->type);
			return;
	}

	stream_close_common(str, result);
}
