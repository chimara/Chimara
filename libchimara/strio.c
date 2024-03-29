#include <errno.h>
#include <string.h>

#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include "charset.h"
#include "magic.h"
#include "stream.h"
#include "ui-message.h"
#include "window.h"

/* Internal function: ensure that an fseek() is called on a file pointer in
 between reading and writing operations, and vice versa. This will only come up
 for ReadWrite or WriteAppend files. */
static void
ensure_file_operation(strid_t str, glui32 op)
{
	if(str->lastop != 0 && str->lastop != op)
	{
		long pos = ftell(str->file_pointer);
		if(pos == -1)
			WARNING_S("ftell() failed", g_strerror(errno));
		if(fseek(str->file_pointer, pos, SEEK_SET) != 0)
			WARNING_S("fseek() failed", g_strerror(errno));
	}
	str->lastop = op; /* Not 0, because we are about to do the operation anyway */
}

/*
 *
 **************** WRITING FUNCTIONS ********************************************
 *
 */

/* Internal function: write a UTF-8 string to a text buffer window's text buffer. */
static void
write_utf8_to_window_buffer(winid_t win, gchar *s)
{
	if(win->input_request_type == INPUT_REQUEST_LINE || win->input_request_type == INPUT_REQUEST_LINE_UNICODE)
	{
		ILLEGAL("Tried to print to a text buffer window with line input pending.");
		return;
	}

	// Write to the buffer	
	g_string_append(win->buffer, s);
}

/* Internal function: flush a window's text buffer to the screen. */
static UiMessage *
flush_window_buffer_internal(winid_t win)
{
	g_debug("%s", win->buffer->str);

	if(win->type != wintype_TextBuffer && win->type != wintype_TextGrid)
		return NULL;

	if(win->buffer->len == 0)
		return NULL;

	UiMessage *msg = ui_message_new(UI_MESSAGE_PRINT_STRING, win);
	msg->strval = g_strdup(win->buffer->str);
	g_string_truncate(win->buffer, 0);
	return msg;
}

/* Queue up a message to print whatever's in @win's text buffer to the screen,
but don't wait until the flush happens. Call this from the Glk thread, for
example, to make sure the current buffer contents are printed before changing
the stream style. */
void
queue_flush_window_buffer(winid_t win)
{
	UiMessage *msg = flush_window_buffer_internal(win);
	if (msg)
		ui_message_queue(msg);
}

/* Queue up a message to print whatever's in @win's text buffer to the screen,
and wait for the queue to catch up. Call this from the Glk thread, for
example, to catch up on all outstanding messages, e.g. in glk_select(). */
void
flush_window_buffer(winid_t win)
{
	UiMessage *msg = flush_window_buffer_internal(win);
	if (msg)
		ui_message_queue_and_await(msg);
}

/* Internal function: write a Latin-1 buffer with length to a stream. */
static void
write_buffer_to_stream(strid_t str, gchar *buf, glui32 len)
{
	switch(str->type)
	{
		case STREAM_TYPE_WINDOW:
			/* Each window type has a different way of printing to it */
			switch(str->window->type)
			{
				/* Printing to these windows' streams does nothing */
				case wintype_Blank:
				case wintype_Pair:
				case wintype_Graphics:
					str->write_count += len;
					break;
					
			    /* Text grid/buffer windows */
			    case wintype_TextGrid:
				{
			        gchar *utf8 = convert_latin1_to_utf8(buf, len);
			        if(utf8 != NULL) {
						/* Deal with newlines */
						int i;
						gchar *line = utf8;
						for(i=0; i<len; i++) {
							if(utf8[i] == '\n') {
								utf8[i] = '\0';
								write_utf8_to_window_buffer(str->window, line);
								ui_message_queue(ui_message_new(UI_MESSAGE_GRID_NEWLINE, str->window));
								line = utf8 + (i < len-1 ? (i+1):(len-1));
							}
						}
								
						/* No more newlines left. */
						write_utf8_to_window_buffer(str->window, line);
						g_free(utf8);
					}

					str->write_count += len;
				}
					break;

				case wintype_TextBuffer:
			    {
			        gchar *utf8 = convert_latin1_to_utf8(buf, len);
			        if(utf8 != NULL) {
						write_utf8_to_window_buffer(str->window, utf8);
						g_free(utf8);
					}
				}	
					str->write_count += len;
					break;
				default:
					ILLEGAL_PARAM("Unknown window type: %u", str->window->type);
			}
			
			/* Now write the same buffer to the window's echo stream */
			if(str->window->echo_stream != NULL)
				write_buffer_to_stream(str->window->echo_stream, buf, len);
			
			break;
			
		case STREAM_TYPE_MEMORY:
			if(str->unicode && str->ubuffer)
			{
				int foo = 0;
				while(str->mark < str->buflen && foo < len)
					str->ubuffer[str->mark++] = (unsigned char)buf[foo++];
			}
			if(!str->unicode && str->buffer)
			{
				int copycount = MIN(len, str->buflen - str->mark);
				memmove(str->buffer + str->mark, buf, copycount);
				str->mark += copycount;
			}

			/* Move the EOF marker if we wrote past it */
			if(str->mark > str->endmark)
				str->endmark = str->mark;

			str->write_count += len;
			break;
			
		case STREAM_TYPE_FILE:
			if(str->binary) 
			{
				if(str->unicode) 
				{
					gchar *writebuffer = convert_latin1_to_ucs4be_string(buf, len);
					ensure_file_operation(str, filemode_Write);
					fwrite(writebuffer, sizeof(gchar), len * 4, str->file_pointer);
					g_free(writebuffer);
				} 
				else /* Regular file */
				{
					ensure_file_operation(str, filemode_Write);
					fwrite(buf, sizeof(gchar), len, str->file_pointer);
				}
			}
			else /* Text mode is the same for Unicode and regular files */
			{
				gchar *utf8 = convert_latin1_to_utf8(buf, len);
				if(utf8 != NULL)
				{
					ensure_file_operation(str, filemode_Write);
					g_fprintf(str->file_pointer, "%s", utf8);
					g_free(utf8);
				}
			}
			
			str->write_count += len;
			break;
		case STREAM_TYPE_RESOURCE:
			ILLEGAL("Writing to a resource stream is illegal.");
			break;
		default:
			ILLEGAL_PARAM("Unknown stream type: %u", str->type);
	}
}

/* Internal function: write a Unicode buffer with length to a stream. */
static void
write_buffer_to_stream_uni(strid_t str, glui32 *buf, glui32 len)
{
	switch(str->type)
	{
		case STREAM_TYPE_WINDOW:
			/* Each window type has a different way of printing to it */
			switch(str->window->type)
			{
				/* Printing to these windows' streams does nothing */
				case wintype_Blank:
				case wintype_Pair:
				case wintype_Graphics:
					str->write_count += len;
					break;
					
			    /* Text grid/buffer windows */
			    case wintype_TextGrid:
			    case wintype_TextBuffer:
			    {
			        gchar *utf8 = convert_ucs4_to_utf8(buf, len);
			        if(utf8 != NULL) {
						write_utf8_to_window_buffer(str->window, utf8);
						g_free(utf8);
					}
				}	
					str->write_count += len;
					break;
				default:
					ILLEGAL_PARAM("Unknown window type: %u", str->window->type);
			}
			
			/* Now write the same buffer to the window's echo stream */
			if(str->window->echo_stream != NULL)
				write_buffer_to_stream_uni(str->window->echo_stream, buf, len);
			
			break;
			
		case STREAM_TYPE_MEMORY:
			if(str->unicode && str->ubuffer)
			{
				int copycount = MIN(len, str->buflen - str->mark);
				memmove(str->ubuffer + str->mark, buf, copycount * sizeof(glui32));
				str->mark += copycount;
			}
			if(!str->unicode && str->buffer)
			{
				gchar *latin1 = convert_ucs4_to_latin1_binary(buf, len);
				int copycount = MIN(len, str->buflen - str->mark);
				memmove(str->buffer + str->mark, latin1, copycount);
				g_free(latin1);
				str->mark += copycount;
			}

			/* Move the EOF marker if we wrote past it */
			if(str->mark > str->endmark)
				str->endmark = str->mark;

			str->write_count += len;
			break;
			
		case STREAM_TYPE_FILE:
			if(str->binary) 
			{
				if(str->unicode) 
				{
					gchar *writebuffer = convert_ucs4_to_ucs4be_string(buf, len);
					ensure_file_operation(str, filemode_Write);
					fwrite(writebuffer, sizeof(gchar), len * 4, str->file_pointer);
					g_free(writebuffer);
				} 
				else /* Regular file */
				{
					gchar *latin1 = convert_ucs4_to_latin1_binary(buf, len);
					ensure_file_operation(str, filemode_Write);
					fwrite(latin1, sizeof(gchar), len, str->file_pointer);
					g_free(latin1);
				}
			}
			else /* Text mode is the same for Unicode and regular files */
			{
				gchar *utf8 = convert_ucs4_to_utf8(buf, len);
				if(utf8 != NULL) 
				{
					ensure_file_operation(str, filemode_Write);
					g_fprintf(str->file_pointer, "%s", utf8);
					g_free(utf8);
				}
			}
			
			str->write_count += len;
			break;
		case STREAM_TYPE_RESOURCE:
			ILLEGAL("Writing to a resource stream is illegal.");
			break;
		default:
			ILLEGAL_PARAM("Unknown stream type: %u", str->type);
	}
}

/**
 * glk_put_char_stream:
 * @str: An output stream.
 * @ch: A character in Latin-1 encoding.
 *
 * The same as glk_put_char(), except that you specify a stream @str to print 
 * to, instead of using the current stream. It is illegal for @str to be %NULL,
 * or an input-only stream.
 */
void
glk_put_char_stream(strid_t str, unsigned char ch)
{
	VALID_STREAM(str, return);
	g_return_if_fail(str->file_mode != filemode_Read);
	
	write_buffer_to_stream(str, (gchar *)&ch, 1);
}

/**
 * glk_put_char_stream_uni:
 * @str: An output stream.
 * @ch: A Unicode code point.
 *
 * The same as glk_put_char_uni(), except that you specify a stream @str to
 * print to, instead of using the current stream. It is illegal for @str to be 
 * %NULL, or an input-only stream.
 */
void
glk_put_char_stream_uni(strid_t str, glui32 ch)
{
	VALID_STREAM(str, return);
	g_return_if_fail(str->file_mode != filemode_Read);
	
	write_buffer_to_stream_uni(str, &ch, 1);
}

/**
 * glk_put_string_stream:
 * @str: An output stream.
 * @s: A null-terminated string in Latin-1 encoding.
 *
 * The same as glk_put_string(), except that you specify a stream @str to print 
 * to, instead of using the current stream. It is illegal for @str to be %NULL,
 * or an input-only stream.
 */
void
glk_put_string_stream(strid_t str, char *s)
{
	VALID_STREAM(str, return);
	if(*s == 0)
		return;

	g_return_if_fail(str->file_mode != filemode_Read);

	write_buffer_to_stream(str, s, strlen(s));
}

/**
 * glk_put_string_stream_uni:
 * @str: An output stream.
 * @s: A null-terminated array of Unicode code points.
 *
 * The same as glk_put_string_uni(), except that you specify a stream @str to
 * print to, instead of using the current stream. It is illegal for @str to be 
 * %NULL, or an input-only stream.
 */
void
glk_put_string_stream_uni(strid_t str, glui32 *s)
{
	VALID_STREAM(str, return);
	if(*s == 0)
		return;

	g_return_if_fail(str->file_mode != filemode_Read);
	
	/* An impromptu strlen() for glui32 arrays */
	glong len = 0;
	glui32 *ptr = s;
	while(*ptr++)
		len++;
	write_buffer_to_stream_uni(str, s, len);
}

/**
 * glk_put_buffer_stream:
 * @str: An output stream.
 * @buf: An array of characters in Latin-1 encoding.
 * @len: Length of @buf.
 *
 * The same as glk_put_buffer(), except that you specify a stream @str to print 
 * to, instead of using the current stream. It is illegal for @str to be %NULL,
 * or an input-only stream.
 */
void
glk_put_buffer_stream(strid_t str, char *buf, glui32 len)
{
	VALID_STREAM(str, return);
	if(len == 0)
		return;

	g_return_if_fail(str->file_mode != filemode_Read);
	
	write_buffer_to_stream(str, buf, len);
}

/**
 * glk_put_buffer_stream_uni:
 * @str: An output stream.
 * @buf: An array of Unicode code points.
 * @len: Length of @buf.
 *
 * The same as glk_put_buffer_uni(), except that you specify a stream @str to
 * print to, instead of using the current stream. It is illegal for @str to be 
 * %NULL, or an input-only stream.
 */
void
glk_put_buffer_stream_uni(strid_t str, glui32 *buf, glui32 len)
{
	VALID_STREAM(str, return);
	if(len == 0)
		return;

	g_return_if_fail(str->file_mode != filemode_Read);
	
	write_buffer_to_stream_uni(str, buf, len);
}

/*
 *
 **************** READING FUNCTIONS ********************************************
 *
 */

/* Internal function: Read one big-endian four-byte character from file fp and
return it as a Unicode code point, or -1 on EOF */
static glsi32
read_ucs4be_char_from_file(strid_t str)
{
	unsigned char readbuffer[4];
	ensure_file_operation(str, filemode_Read);
	if(fread(readbuffer, sizeof(unsigned char), 4, str->file_pointer) < 4)
		return -1; /* EOF */
	return
		readbuffer[0] << 24 | 
		readbuffer[1] << 16 | 
		readbuffer[2] << 8  | 
		readbuffer[3];
}

/* Internal function: Read one UTF-8 character, which may be more than one byte,
from file fp and return it as a Unicode code point, or -1 on EOF */
static glsi32
read_utf8_char_from_file(strid_t str)
{
	gchar readbuffer[4] = {0, 0, 0, 0}; /* Max UTF-8 width */
	int foo;
	gunichar charresult = (gunichar)-2;
	ensure_file_operation(str, filemode_Read);
	for(foo = 0; foo < 4 && charresult == (gunichar)-2; foo++) 
	{
		int ch = fgetc(str->file_pointer);
		if(ch == EOF)
			return -1;
		readbuffer[foo] = (gchar)ch;
		charresult = g_utf8_get_char_validated(readbuffer, foo + 1);
		/* charresult is -1 if invalid, -2 if incomplete, and the unicode code
		point otherwise */
	}
	/* Silently return unknown characters as 0xFFFD, Replacement Character */
	if(charresult == (gunichar)-1 || charresult == (gunichar)-2) 
		return 0xFFFD;
	return charresult;
}

/* Internal function: Read one UTF-8 character, which may be more than one byte,
from a memory stream @str, and return it as a Unicode code point. */
static glsi32
read_utf8_char_from_buffer(strid_t str)
{
	size_t foo;
	gunichar charresult = (gunichar)-2;
	char *buffer = str->buffer + str->mark;
	size_t maxlen = str->buflen - str->mark;

	if(maxlen == 0)
		return -1;

	for(foo = 1; foo <= maxlen; foo++)
	{
		charresult = g_utf8_get_char_validated(buffer, foo);
		/* charresult is -1 if invalid, -2 if incomplete, and the
		Unicode code point otherwise */
		if(charresult != (gunichar)-2)
			break;
	}
	str->mark += foo;
	str->read_count++;

	/* Return -1 on EOS */
	if(charresult == (gunichar)-2)
		return -1;
	/* Silently return unknown characters as 0xFFFD, Replacement Character */
	if(charresult == (gunichar)-1)
		return 0xFFFD;
	return charresult;
}

/* Internal function: Read one big-endian four-byte character from memory and
return it as a Unicode code point, or -1 on EOF */
static glsi32
read_ucs4be_char_from_buffer(strid_t str)
{
	glui32 ch = str->buffer[str->mark++];
	if(str->mark >= str->buflen)
		return -1;
	ch = (ch << 8) | (str->buffer[str->mark++] & 0xFF);
	if(str->mark >= str->buflen)
		return -1;
	ch = (ch << 8) | (str->buffer[str->mark++] & 0xFF);
	if(str->mark >= str->buflen)
		return -1;
	ch = (ch << 8) | (str->buffer[str->mark++] & 0xFF);
	str->read_count++;
	return ch;
}

/* Internal function: Tell whether this code point is a Unicode newline. The
file pointer and eight-bit flag are included in case the newline is a CR 
(U+000D). If the next character is LF (U+000A) then it also belongs to the
newline. */
static gboolean
is_unicode_newline(glsi32 ch, strid_t str, gboolean utf8)
{
	if(ch == 0x0A || ch == 0x85 || ch == 0x0C || ch == 0x2028 || ch == 0x2029)
		return TRUE;
	if(ch == 0x0D) {
		glsi32 ch2 = utf8? read_utf8_char_from_file(str) :
			read_ucs4be_char_from_file(str);
		if(ch2 != 0x0A) {
			if(fseek(str->file_pointer, utf8? -1 : -4, SEEK_CUR) == -1)
				WARNING_S("Seek failed on stream", g_strerror(errno) );
			str->lastop = 0; /* can read or write after a seek */
		}
		return TRUE;
	}
	return FALSE;
}

/* Internal function: Read one character from a stream. Returns a value which
 can be returned unchanged by glk_get_char_stream_uni(), but 
 glk_get_char_stream() must replace high values by the placeholder character. */
static glsi32
get_char_stream_common(strid_t str)
{
	switch(str->type)
	{
		case STREAM_TYPE_RESOURCE:
			if(str->unicode)
			{
				if(!str->buffer || str->mark >= str->buflen)
					return -1;
				if(str->binary)
					/* Cheap big-endian stream */
					return read_ucs4be_char_from_buffer(str);
				/* slightly less cheap UTF8 stream */
				return read_utf8_char_from_buffer(str);
			}
			/* for text streams, fall through to memory case */
		case STREAM_TYPE_MEMORY:
			if(str->unicode)
			{
				if(!str->ubuffer || str->mark >= str->buflen)
					return -1;
				glui32 ch = str->ubuffer[str->mark++];
				str->read_count++;
				return ch;
			}
			else
			{
				if(!str->buffer || str->mark >= str->buflen)
					return -1;
				unsigned char ch = str->buffer[str->mark++];
				str->read_count++;
				return ch;
			}
			break;
			
		case STREAM_TYPE_FILE:
			if(str->binary) 
			{
				if(str->unicode) 
				{
					glsi32 ch = read_ucs4be_char_from_file(str);
					if(ch == -1)
						return -1;
					str->read_count++;
					return ch;
				}
				else /* Regular file */
				{
					ensure_file_operation(str, filemode_Read);
					int ch = fgetc(str->file_pointer);
					if(ch == EOF)
						return -1;
					
					str->read_count++;
					return ch;
				}
			}
			else /* Text mode is the same for Unicode and regular files */
			{
				glsi32 ch = read_utf8_char_from_file(str);
				if(ch == -1)
					return -1;
					
				str->read_count++;
				return ch;
			}
		default:
			ILLEGAL_PARAM("Reading illegal on stream type: %u", str->type);
			return -1;
	}
}

/**
 * glk_get_char_stream:
 * @str: An input stream.
 *
 * Reads one character from the stream @str.
 * (There is no notion of a “current input stream.”)
 * It is illegal for @str to be %NULL, or an output-only stream.
 *
 * The result will be between 0 and 255. As with all basic text functions, Glk
 * assumes the Latin-1 encoding.
 * See [Character Encoding][chimara-Character-Encoding].
 * If the end of the stream has been reached, the result will be -1.
 *
 * <note><para>
 *   Note that high-bit characters (128..255) are <emphasis>not</emphasis>
 *   returned as negative numbers.
 * </para></note>
 *
 * If the stream contains Unicode data &mdash; for example, if it was created
 * with glk_stream_open_file_uni() or glk_stream_open_memory_uni() &mdash; then
 * characters beyond 255 will be returned as 0x3F (`"?"`).
 *
 * It is usually more efficient to read several characters at once with
 * glk_get_buffer_stream() or glk_get_line_stream(), as opposed to calling
 * glk_get_char_stream() several times.
 *
 * Returns: A character value between 0 and 255, or -1 on end of stream.
 */
glsi32
glk_get_char_stream(strid_t str)
{
	VALID_STREAM(str, return -1);
	g_return_val_if_fail(str->file_mode == filemode_Read || str->file_mode == filemode_ReadWrite, -1);
	
	glsi32 ch = get_char_stream_common(str);
	return (ch > 0xFF)? PLACEHOLDER : ch;
}

/**
 * glk_get_char_stream_uni:
 * @str: An input stream.
 *
 * Reads one character from the stream @str. If the end of the stream has been
 * reached, the result will be -1.
 *
 * Returns: A value between 0 and 0x7FFFFFFF, or -1 on end of stream.
 */
glsi32
glk_get_char_stream_uni(strid_t str)
{
	VALID_STREAM(str, return -1);
	g_return_val_if_fail(str->file_mode == filemode_Read || str->file_mode == filemode_ReadWrite, -1);
	
	return get_char_stream_common(str);
}

/**
 * glk_get_buffer_stream:
 * @str: An input stream.
 * @buf: A buffer with space for at least @len characters.
 * @len: The number of characters to read.
 *
 * Reads @len characters from @str, unless the end of stream is reached first.
 * No terminal null is placed in the buffer.
 *
 * Returns: The number of characters actually read.
 */
glui32
glk_get_buffer_stream(strid_t str, char *buf, glui32 len)
{
	VALID_STREAM(str, return 0);
	g_return_val_if_fail(str->file_mode == filemode_Read || str->file_mode == filemode_ReadWrite, 0);
	g_return_val_if_fail(buf != NULL, 0);
	
	switch(str->type)
	{
		case STREAM_TYPE_RESOURCE:
		{
			int copycount = 0;
			if(str->unicode)
			{
				while(copycount < len && str->buffer && str->mark < str->buflen)
				{
					glui32 ch;
					if(str->binary)
						ch = read_ucs4be_char_from_buffer(str);
					else
						ch = read_utf8_char_from_buffer(str);
					buf[copycount++] = (ch > 0xFF)? '?' : (char)ch;
				}
				return copycount;
			}
			/* for text streams, fall through to memory case */
		}
		case STREAM_TYPE_MEMORY:
		{
			int copycount = 0;
			if(str->unicode)
			{
				while(copycount < len && str->ubuffer && str->mark < str->buflen) 
				{
					glui32 ch = str->ubuffer[str->mark++];
					buf[copycount++] = (ch > 0xFF)? '?' : (char)ch;
				}
			}
			else
			{
				if(str->buffer) /* if not, copycount stays 0 */
					copycount = MIN(len, str->buflen - str->mark);
				memmove(buf, str->buffer + str->mark, copycount);
				str->mark += copycount;
			}

			str->read_count += copycount;		
			return copycount;
		}	
		case STREAM_TYPE_FILE:
			if(str->binary) 
			{
				if(str->unicode) /* Binary file with 4-byte characters */
				{
					/* Read len characters of 4 bytes each */
					unsigned char *readbuffer = g_new0(unsigned char, 4 * len);
					ensure_file_operation(str, filemode_Read);
					size_t count = fread(readbuffer, sizeof(unsigned char), 4 * len, str->file_pointer);
					/* If there was an incomplete character */
					if(count % 4 != 0) 
					{
						count -= count % 4;
						WARNING("Incomplete character in binary Unicode file");
					}
					
					int foo;
					for(foo = 0; foo < count; foo += 4)
					{
						glsi32 ch = readbuffer[foo] << 24
							| readbuffer[foo + 1] << 16
							| readbuffer[foo + 2] << 8
							| readbuffer[foo + 3];
						buf[foo / 4] = (ch > 255)? 0x3F : (char)ch;
					}
					g_free(readbuffer);
					str->read_count += count / 4;
					return count / 4;
				}
				else /* Regular binary file */
				{
					ensure_file_operation(str, filemode_Read);
					size_t count = fread(buf, sizeof(char), len, str->file_pointer);
					str->read_count += count;
					return count;
				}
			}
			else /* Text mode is the same for Unicode and regular files */
			{
				/* Do it character-by-character */
				int foo;
				for(foo = 0; foo < len; foo++)
				{
					glsi32 ch = read_utf8_char_from_file(str);
					if(ch == -1)
						break;
					str->read_count++;
					buf[foo] = (ch > 0xFF)? 0x3F : (gchar)ch;
				}
				return foo;
			}
		default:
			ILLEGAL_PARAM("Reading illegal on stream type: %u", str->type);
			return 0;
	}
}

/**
 * glk_get_buffer_stream_uni:
 * @str: An input stream.
 * @buf: A buffer with space for at least @len Unicode code points.
 * @len: The number of characters to read.
 *
 * Reads @len Unicode characters from @str, unless the end of stream is reached 
 * first. No terminal null is placed in the buffer.
 *
 * Returns: The number of Unicode characters actually read.
 */
glui32
glk_get_buffer_stream_uni(strid_t str, glui32 *buf, glui32 len)
{
	VALID_STREAM(str, return 0);
	g_return_val_if_fail(str->file_mode == filemode_Read || str->file_mode == filemode_ReadWrite, 0);
	g_return_val_if_fail(buf != NULL, 0);
	
	switch(str->type)
	{
		case STREAM_TYPE_RESOURCE:
		{
			int copycount = 0;
			if(str->unicode)
			{
				while(copycount < len && str->buffer && str->mark < str->buflen)
				{
					glui32 ch;
					if(str->binary)
						ch = read_ucs4be_char_from_buffer(str);
					else
						ch = read_utf8_char_from_buffer(str);
					buf[copycount++] = ch;
				}
				return copycount;
			}
			/* for text streams, fall through to memory case */
		}
		case STREAM_TYPE_MEMORY:
		{
			int copycount = 0;
			if(str->unicode)
			{
				if(str->ubuffer) /* if not, copycount stays 0 */
					copycount = MIN(len, str->buflen - str->mark);
				memmove(buf, str->ubuffer + str->mark, copycount * 4);
				str->mark += copycount;
			}
			else
			{
				while(copycount < len && str->buffer && str->mark < str->buflen)
				{
					unsigned char ch = str->buffer[str->mark++];
					buf[copycount++] = ch;
				}
			}

			str->read_count += copycount;		
			return copycount;
		}	
		case STREAM_TYPE_FILE:
			if(str->binary) 
			{
				if(str->unicode) /* Binary file with 4-byte characters */
				{
					/* Read len characters of 4 bytes each */
					unsigned char *readbuffer = g_new0(unsigned char, 4 * len);
					ensure_file_operation(str, filemode_Read);
					size_t count = fread(readbuffer, sizeof(unsigned char), 4 * len, str->file_pointer);
					/* If there was an incomplete character */
					if(count % 4 != 0) 
					{
						count -= count % 4;
						WARNING("Incomplete character in binary Unicode file");
					}
					
					int foo;
					for(foo = 0; foo < count; foo += 4)
						buf[foo / 4] = readbuffer[foo] << 24
							| readbuffer[foo + 1] << 16
							| readbuffer[foo + 2] << 8
							| readbuffer[foo + 3];
					g_free(readbuffer);
					str->read_count += count / 4;
					return count / 4;
				}
				else /* Regular binary file */
				{
					unsigned char *readbuffer = g_new0(unsigned char, len);
					ensure_file_operation(str, filemode_Read);
					size_t count = fread(readbuffer, sizeof(unsigned char), len, str->file_pointer);
					int foo;
					for(foo = 0; foo < count; foo++)
						buf[foo] = readbuffer[foo];
					g_free(readbuffer);
					str->read_count += count;
					return count;
				}
			}
			else /* Text mode is the same for Unicode and regular files */
			{
				/* Do it character-by-character */
				int foo;
				for(foo = 0; foo < len; foo++)
				{
					glsi32 ch = read_utf8_char_from_file(str);
					if(ch == -1)
						break;
					str->read_count++;
					buf[foo] = ch;
				}
				return foo;
			}
		default:
			ILLEGAL_PARAM("Reading illegal on stream type: %u", str->type);
			return 0;
	}
}

/**
 * glk_get_line_stream:
 * @str: An input stream.
 * @buf: A buffer with space for at least @len characters.
 * @len: The number of characters to read, plus one.
 *
 * Reads characters from @str, until either <inlineequation><alt>@len -
 * 1</alt><mathphrase>@len - 1</mathphrase></inlineequation> characters have
 * been read or a newline has been read.
 * It then puts a terminal null (`'\0'`) character on the end.
 * It returns the number of characters actually read, including the newline (if
 * there is one) but not including the terminal null.
 *
 * Returns: The number of characters actually read.
 */
glui32
glk_get_line_stream(strid_t str, char *buf, glui32 len)
{
	VALID_STREAM(str, return 0);
	g_return_val_if_fail(str->file_mode == filemode_Read || str->file_mode == filemode_ReadWrite, 0);
	g_return_val_if_fail(buf != NULL, 0);

	switch(str->type)
	{
		case STREAM_TYPE_RESOURCE:
		{
			int copycount = 0;
			if(str->unicode)
			{
				/* Do it character-by-character */
				while(copycount < len - 1 && str->buffer && str->mark < str->buflen)
				{
					glsi32 ch;
					if(str->binary)
						ch = read_ucs4be_char_from_buffer(str);
					else
						ch = read_utf8_char_from_buffer(str);
					/* Check for Unicode newline; slightly different than
					in file streams */
					if(ch == 0x0A || ch == 0x85 || ch == 0x0C || ch == 0x2028 || ch == 0x2029)
					{
						buf[copycount++] = '\n';
						break;
					}
					if(ch == 0x0D)
					{
						if(str->buffer[str->mark] == 0x0A)
							str->mark++; /* skip past next newline */
						buf[copycount++] = '\n';
						break;
					}
					buf[copycount++] = (ch > 0xFF)? '?' : (char)ch;
				}
				buf[copycount] = '\0';
				return copycount;
			}
			/* for text streams, fall through to the memory case */
		}
		case STREAM_TYPE_MEMORY:
		{
			int copycount = 0;
			if(str->unicode)
			{
				/* Do it character-by-character */
				while(copycount < len - 1 && str->ubuffer && str->mark < str->buflen) 
				{
					glui32 ch = str->ubuffer[str->mark++];
					/* Check for Unicode newline; slightly different than
					in file streams */
					if(ch == 0x0A || ch == 0x85 || ch == 0x0C || ch == 0x2028 || ch == 0x2029)
					{
						buf[copycount++] = '\n';
						break;
					}
					if(ch == 0x0D)
					{
						if(str->ubuffer[str->mark] == 0x0A)
							str->mark++; /* skip past next newline */
						buf[copycount++] = '\n';
						break;
					}
					buf[copycount++] = (ch > 0xFF)? '?' : (char)ch;
				}
				buf[copycount] = '\0';
			}
			else
			{
				if(str->buffer) /* if not, copycount stays 0 */
					copycount = MIN(len - 1, str->buflen - str->mark);
				char *endptr = memccpy(buf, str->buffer + str->mark, '\n', copycount);
				if(endptr) /* newline was found */
					copycount = endptr - buf; /* Real copy count */
				buf[copycount] = '\0';
				str->mark += copycount;
			}
			
			str->read_count += copycount;
			return copycount;
		}	
		case STREAM_TYPE_FILE:
			if(str->binary) 
			{
				if(str->unicode) /* Binary file with 4-byte characters */
				{
					/* Do it character-by-character */
					int copycount;
					for(copycount = 0; copycount < len - 1; copycount++)
					{
						glsi32 ch = read_ucs4be_char_from_file(str);
						if(ch == -1) 
						{
							buf[copycount] = '\0';
							return copycount;
						}
						str->read_count++;
						if(is_unicode_newline(ch, str, FALSE))
						{
							buf[copycount++] = '\n';
							buf[copycount] = '\0';
							return copycount;
						}
						buf[copycount] = (ch > 0xFF)? '?' : (char)ch;
					}
					buf[len] = '\0';
					return copycount;
				}
				else /* Regular binary file */
				{
					ensure_file_operation(str, filemode_Read);
					if( !fgets(buf, len, str->file_pointer) ) {
						*buf = 0;
						return 0;
					}

					int nread = strlen(buf);
					str->read_count += nread;
					return nread;
				}
			}
			else /* Text mode is the same for Unicode and regular files */
			{
				/* Do it character-by-character */
				int foo;
				for(foo = 0; foo < len - 1; foo++)
				{
					glsi32 ch = read_utf8_char_from_file(str);
					if(ch == -1)
					{
						buf[foo] = '\0';
						return foo - 1;
					}
					str->read_count++;
					if(is_unicode_newline(ch, str, TRUE))
					{
						buf[foo] = '\n';
						buf[foo + 1] = '\0';
						return foo;
					}
					buf[foo] = (ch > 0xFF)? 0x3F : (char)ch;
				}
				buf[len] = '\0';
				return foo;
			}
		default:
			ILLEGAL_PARAM("Reading illegal on stream type: %u", str->type);
			return 0;
	}
}

/**
 * glk_get_line_stream_uni:
 * @str: An input stream.
 * @buf: A buffer with space for at least @len Unicode code points.
 * @len: The number of characters to read, plus one.
 *
 * Reads Unicode characters from @str, until either <inlineequation><alt>@len -
 * 1</alt><mathphrase>@len - 1</mathphrase></inlineequation> Unicode characters
 * have been read or a newline has been read.
 * It then puts a terminal null (a zero value) on the end.
 *
 * Returns: The number of characters actually read, including the newline (if
 * there is one) but not including the terminal null.
 */
glui32
glk_get_line_stream_uni(strid_t str, glui32 *buf, glui32 len)
{
	VALID_STREAM(str, return 0);
	g_return_val_if_fail(str->file_mode == filemode_Read || str->file_mode == filemode_ReadWrite, 0);
	g_return_val_if_fail(buf != NULL, 0);

	switch(str->type)
	{
		case STREAM_TYPE_RESOURCE:
		{
			int copycount = 0;
			if(str->unicode)
			{
				/* Do it character-by-character */
				while(copycount < len - 1 && str->buffer && str->mark < str->buflen) 
				{
					glsi32 ch;
					if(str->binary)
						ch = read_ucs4be_char_from_buffer(str);
					else
						ch = read_utf8_char_from_buffer(str);
					/* Check for Unicode newline; slightly different than
					in file streams */
					if(ch == 0x0A || ch == 0x85 || ch == 0x0C || ch == 0x2028 || ch == 0x2029)
					{
						buf[copycount++] = '\n';
						break;
					}
					if(ch == 0x0D)
					{
						if(str->ubuffer[str->mark] == 0x0A)
							str->mark++; /* skip past next newline */
						buf[copycount++] = '\n';
						break;
					}
					buf[copycount++] = ch;
				}
				buf[copycount] = '\0';
				return copycount;
			}
			/* for text streams, fall through to the memory case */
		}
		case STREAM_TYPE_MEMORY:
		{
			int copycount = 0;
			if(str->unicode)
			{
				/* Do it character-by-character */
				while(copycount < len - 1 && str->ubuffer && str->mark < str->buflen) 
				{
					glui32 ch = str->ubuffer[str->mark++];
					/* Check for Unicode newline; slightly different than
					in file streams */
					if(ch == 0x0A || ch == 0x85 || ch == 0x0C || ch == 0x2028 || ch == 0x2029)
					{
						buf[copycount++] = '\n';
						break;
					}
					if(ch == 0x0D)
					{
						if(str->ubuffer[str->mark] == 0x0A)
							str->mark++; /* skip past next newline */
						buf[copycount++] = '\n';
						break;
					}
					buf[copycount++] = ch;
				}
				buf[copycount] = '\0';
			}
			else
			{
				/* No recourse to memccpy(), so do it character-by-character */
				while(copycount < len - 1 && str->buffer && str->mark < str->buflen)
				{
					gchar ch = str->buffer[str->mark++];
					/* Check for newline */
					if(ch == '\n') /* Also check for \r and \r\n? */
					{
						buf[copycount++] = '\n';
						break;
					}
					buf[copycount++] = (unsigned char)ch;
				}
				buf[copycount] = 0;
			}
			
			str->read_count += copycount;
			return copycount;
		}	
		case STREAM_TYPE_FILE:
			if(str->binary) 
			{
				if(str->unicode) /* Binary file with 4-byte characters */
				{
					/* Do it character-by-character */
					int copycount;
					for(copycount = 0; copycount < len - 1; copycount++)
					{
						glsi32 ch = read_ucs4be_char_from_file(str);
						if(ch == -1) 
						{
							buf[copycount] = 0;
							return copycount;
						}
						str->read_count++;
						if(is_unicode_newline(ch, str, FALSE))
						{
							buf[copycount++] = ch; /* Preserve newline types??? */
							buf[copycount] = 0;
							return copycount;
						}
						buf[copycount] = ch;
					}
					buf[len] = 0;
					return copycount;
				}
				else /* Regular binary file */
				{
					g_autofree char *readbuffer = g_new0(char, len);
					ensure_file_operation(str, filemode_Read);
					if( !fgets(readbuffer, len, str->file_pointer) ) {
						*buf = 0;
						return 0;
					}

					glui32 count = strlen(readbuffer);
					int foo;
					for(foo = 0; foo < count + 1; foo++) /* Copy terminator */
						buf[foo] = (unsigned char)(readbuffer[foo]);
					str->read_count += count;
					return count;
				}
			}
			else /* Text mode is the same for Unicode and regular files */
			{
				/* Do it character-by-character */
				int foo;
				for(foo = 0; foo < len - 1; foo++)
				{
					glsi32 ch = read_utf8_char_from_file(str);
					if(ch == -1)
					{
						buf[foo] = 0;
						return foo - 1;
					}
					str->read_count++;
					if(is_unicode_newline(ch, str, TRUE))
					{
						buf[foo] = ch; /* Preserve newline types??? */
						buf[foo + 1] = 0;
						return foo;
					}
					buf[foo] = ch;
				}
				buf[len] = 0;
				return foo;
			}
		default:
			ILLEGAL_PARAM("Reading illegal on stream type: %u", str->type);
			return 0;
	}
}

/*
 *
 **************** SEEKING FUNCTIONS ********************************************
 *
 */

/**
 * glk_stream_get_position:
 * @str: A file or memory stream.
 *
 * Returns the position of the read/write mark in @str. For memory streams and
 * binary file streams, this is exactly the number of characters read or written
 * from the beginning of the stream (unless you have moved the mark with
 * glk_stream_set_position().) For text file streams, matters are more 
 * ambiguous, since (for example) writing one byte to a text file may store more
 * than one character in the platform's native encoding. You can only be sure
 * that the position increases as you read or write to the file.
 *
 * Additional complication: for Latin-1 memory and file streams, a character is
 * a byte. For Unicode memory and file streams (those created by
 * glk_stream_open_file_uni() and glk_stream_open_memory_uni()), a character is
 * a 32-bit word. So in a binary Unicode file, positions are multiples of four
 * bytes.
 *
 * <note><para>
 *   If this bothers you, don't use binary Unicode files. I don't think they're
 *   good for much anyhow.
 * </para></note>
 *
 * `glk_stream_get_position()` on a window stream will always return zero.
 *
 * <note><para>
 *   It might make more sense to return the number of characters written to the
 *   window, but existing libraries do not support this and it's not really
 *   worth adding the feature.
 * </para></note>
 *
 * Returns: position of the read/write mark in @str.
 */
glui32
glk_stream_get_position(strid_t str)
{
	VALID_STREAM(str, return 0);
	
	switch(str->type)
	{
		case STREAM_TYPE_MEMORY:
		case STREAM_TYPE_RESOURCE:
			return str->mark;
		case STREAM_TYPE_FILE:
			return ftell(str->file_pointer);
		case STREAM_TYPE_WINDOW:
			return 0;
		default:
			ILLEGAL_PARAM("Seeking illegal on stream type: %u", str->type);
			return 0;
	}
}

/**
 * glk_stream_set_position:
 * @str: A file or memory stream.
 * @pos: The position to set the mark to, relative to @seekmode.
 * @seekmode: One of %seekmode_Start, %seekmode_Current, or %seekmode_End.
 *
 * Sets the position of the read/write mark in @str. The position is controlled
 * by @pos, and the meaning of @pos is controlled by @seekmode. See the
 * `seekmode_` constants below.
 *
 * It is illegal to specify a position before the beginning or after the end of
 * the file.
 *
 * In binary files, the mark position is exact &mdash; it corresponds with the
 * number of characters you have read or written. In text files, this mapping 
 * can vary, because of linefeed conventions or other character-set 
 * approximations.
 * See [Streams][chimara-Streams].
 * glk_stream_set_position() and glk_stream_get_position() measure positions in
 * the platform's native encoding &mdash; after character cookery. Therefore,
 * in a text stream, it is safest to use glk_stream_set_position() only to move
 * to the beginning or end of a file, or to a position determined by
 * glk_stream_get_position().
 *
 * Again, in Latin-1 streams, characters are bytes. In Unicode streams,
 * characters are 32-bit words, or four bytes each.
 *
 * A window stream doesn't have a movable mark, so calling
 * glk_stream_set_position() has no effect.
 */
void
glk_stream_set_position(strid_t str, glsi32 pos, glui32 seekmode)
{
	VALID_STREAM(str, return);
	g_return_if_fail(!(seekmode == seekmode_Start && pos < 0));
	g_return_if_fail(!(seekmode == seekmode_End && pos > 0));
	
	switch(str->type)
	{
		case STREAM_TYPE_RESOURCE:
		case STREAM_TYPE_MEMORY:
			switch(seekmode)
			{
				case seekmode_Start:   str->mark = pos;  break;
				case seekmode_Current: str->mark += pos; break;
				case seekmode_End:     str->mark = str->endmark + pos; break;
				default:
					g_return_if_reached();
					return;
			}
			break;
		case STREAM_TYPE_FILE:
		{
			int whence;
			switch(seekmode)
			{
				case seekmode_Start:   whence = SEEK_SET; break;
				case seekmode_Current: whence = SEEK_CUR; break;
				case seekmode_End:     whence = SEEK_END; break;
				default:
					g_return_if_reached();
					return;
			}
			if(fseek(str->file_pointer, pos, whence) == -1)
				WARNING("Seek failed on file stream");
			str->lastop = 0; /* Either reading or writing is legal after fseek() */
			break;
		}
		case STREAM_TYPE_WINDOW:
			break; /* Quietly do nothing */
		default:
			ILLEGAL_PARAM("Seeking illegal on stream type: %u", str->type);
			return;
	}
}

