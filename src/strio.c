#include "stream.h"
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>

#define min(x,y) ( (x > y)? y : x )

/*
 *
 **************** WRITING FUNCTIONS ********************************************
 *
 */

/* Internal function: change illegal (control) characters in a string to a
placeholder character. Must free returned string afterwards. */
static gchar *
remove_latin1_control_characters(unsigned char *s, gssize len)
{
	gchar *retval = g_new0(gchar, len);
	int i;
	for(i = 0; i < len; i++)
		if( (s[i] < 32 && s[i] != 10) || (s[i] >= 127 && s[i] <= 159) )
			retval[i] = '?';
			/* Our placeholder character is '?'; other options are possible,
			like printing "0x7F" or something */
		else
			retval[i] = s[i];
	return retval;
}

/* Internal function: convert a Latin-1 string to a UTF-8 string, replacing
Latin-1 control characters by a placeholder first. The UTF-8 string must be
freed afterwards. Returns NULL on error. */
static gchar *
convert_latin1_to_utf8(gchar *s, gssize len)
{
	GError *error = NULL;
	gchar *utf8;
	gchar *canonical = remove_latin1_control_characters( (unsigned char *)s,
		len);
	utf8 = g_convert(canonical, len, "UTF-8", "ISO-8859-1", NULL, NULL, &error);
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

/* Internal function: write a UTF-8 buffer with length to a stream. */
static void
write_buffer_to_stream(strid_t str, gchar *buf, glui32 len)
{
	switch(str->stream_type)
	{
		case STREAM_TYPE_WINDOW:
			/* Each window type has a different way of printing to it */
			switch(str->window->window_type)
			{
				/* Printing to these windows' streams does nothing */
				case wintype_Blank:
				case wintype_Pair:
				case wintype_Graphics:
					str->write_count += len;
					break;
				/* Text buffer window */	
				case wintype_TextBuffer:
				{
					gchar *utf8 = convert_latin1_to_utf8(buf, len);
					write_utf8_to_window(str->window, utf8);
					g_free(utf8);
				}	
					str->write_count += len;
					break;
				default:
					g_warning("%s: Writing to this kind of window unsupported.",
						__func__);
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
					str->ubuffer[str->mark++] = (glui32)buf[foo++];
			}
			if(!str->unicode && str->buffer)
			{
				memmove(str->buffer + str->mark, buf, 
					min(len, str->buflen - str->mark));
			}

			str->write_count += len;
			break;
			
		case STREAM_TYPE_FILE:
			if(str->binary) 
			{
				if(str->unicode) 
				{
					/* Convert to four-byte big-endian */
					gchar *writebuffer = g_new0(gchar, len * 4);
					int i;
					for(i = 0; i < len; i++)
						writebuffer[i * 4 + 3] = buf[i];
					fwrite(writebuffer, sizeof(gchar), len * 4, 
						str->file_pointer);
				} 
				else /* Regular file */
				{
					fwrite(buf, sizeof(gchar), len, str->file_pointer);
				}
			}
			else /* Text mode is the same for Unicode and regular files */
			{
				gchar *utf8 = convert_latin1_to_utf8(buf, len);
				g_fprintf(str->file_pointer, "%s", utf8);
				g_free(utf8);
			}
			
			str->write_count += len;
			break;
		default:
			g_warning("%s: Writing to this kind of stream unsupported.",
				__func__);
	}
}

/**
 * glk_put_char_stream:
 * @str: An output stream.
 * @ch: A character in Latin-1 encoding.
 *
 * Prints one character @ch to the stream @str. It is illegal for @str to be
 * %NULL, or an input-only stream.
 */
void
glk_put_char_stream(strid_t str, unsigned char ch)
{
	g_return_if_fail(str != NULL);
	g_return_if_fail(str->file_mode != filemode_Read);
	
	write_buffer_to_stream(str, (gchar *)&ch, 1);
}

/**
 * glk_put_string_stream:
 * @str: An output stream.
 * @s: A null-terminated string in Latin-1 encoding.
 *
 * Prints @s to the stream @str. It is illegal for @str to be %NULL, or an
 * input-only stream.
 */
void
glk_put_string_stream(strid_t str, char *s)
{
	g_return_if_fail(str != NULL);
	g_return_if_fail(str->file_mode != filemode_Read);

	write_buffer_to_stream(str, (gchar *)s, strlen(s));
}

/**
 * glk_put_buffer_stream:
 * @str: An output stream.
 * @buf: An array of characters in Latin-1 encoding.
 * @len: Length of @buf.
 *
 * Prints @buf to the stream @str. It is illegal for @str to be %NULL, or an
 * input-only stream.
 */
void
glk_put_buffer_stream(strid_t str, char *buf, glui32 len)
{
	g_return_if_fail(str != NULL);
	g_return_if_fail(str->file_mode != filemode_Read);
	
	write_buffer_to_stream(str, (gchar *)buf, len);
}

/*
 *
 **************** READING FUNCTIONS ********************************************
 *
 */

/* Internal function: Read one big-endian four-byte character from file fp and
return it as a Unicode code point, or -1 on EOF */
static glsi32
read_ucs4be_char_from_file(FILE *fp)
{
	unsigned char readbuffer[4];
	if(fread(readbuffer, sizeof(unsigned char), 4, fp) < 4)
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
read_utf8_char_from_file(FILE *fp)
{
	gchar readbuffer[4] = {0, 0, 0, 0}; /* Max UTF-8 width */
	int foo;
	gunichar charresult = (gunichar)-2;
	for(foo = 0; foo < 4 && charresult == (gunichar)-2; foo++) 
	{
		int ch = fgetc(fp);
		if(ch == EOF)
			return -1;
		readbuffer[foo] = (gchar)ch;
		charresult = g_utf8_get_char_validated(readbuffer, foo);
		/* charresult is -1 if invalid, -2 if incomplete, and the unicode code
		point otherwise */
	}
	/* Silently return unknown characters as 0xFFFD, Replacement Character */
	if(charresult == (gunichar)-1 || charresult == (gunichar)-2) 
		return 0xFFFD;
	return charresult;
}

/* Internal function: Tell whether this code point is a Unicode newline. The
file pointer and eight-bit flag are included in case the newline is a CR 
(U+000D). If the next character is LF (U+000A) then it also belongs to the
newline. */
static gboolean
is_unicode_newline(glsi32 ch, FILE *fp, gboolean utf8)
{
	if(ch == 0x0A || ch == 0x85 || ch == 0x0C || ch == 0x2028 || ch == 0x2029)
		return TRUE;
	if(ch == 0x0D) {
		glsi32 ch2 = utf8? read_utf8_char_from_file(fp) : 
			read_ucs4be_char_from_file(fp);
		if(ch2 != 0x0A)
			fseek(fp, utf8? -1 : -4, SEEK_CUR);
		return TRUE;
	}
	return FALSE;
}

/**
 * glk_get_char_stream:
 * @str: An input stream.
 *
 * Reads one character from the stream @str. (There is no notion of a ``current
 * input stream.'') It is illegal for @str to be %NULL, or an output-only
 * stream.
 *
 * The result will be between 0 and 255. As with all basic text functions, Glk
 * assumes the Latin-1 encoding. If the end of the stream has been reached, the
 * result will be -1. Note that high-bit characters (128..255) are
 * <emphasis>not</emphasis> returned as negative numbers.
 *
 * If the stream contains Unicode data --- for example, if it was created with
 * glk_stream_open_file_uni() or glk_stream_open_memory_uni() --- then
 * characters beyond 255 will be returned as 0x3F ("?").
 *
 * Returns: A character value between 0 and 255, or -1 on end of stream.
 */
glsi32
glk_get_char_stream(strid_t str)
{
	g_return_val_if_fail(str != NULL, -1);
	g_return_val_if_fail(str->file_mode == filemode_Read
		|| str->file_mode == filemode_ReadWrite, -1);
	
	switch(str->stream_type)
	{
		case STREAM_TYPE_MEMORY:
			if(str->unicode)
			{
				if(!str->ubuffer || str->mark >= str->buflen)
					return -1;
				glui32 ch = str->ubuffer[str->mark++];
				str->read_count++;
				return (ch > 0xFF)? 0x3F : ch;
			}
			else
			{
				if(!str->buffer || str->mark >= str->buflen)
					return -1;
				char ch = str->buffer[str->mark++];
				str->read_count++;
				return ch;
			}
			break;
			
		case STREAM_TYPE_FILE:
			if(str->binary) 
			{
				if(str->unicode) 
				{
					glsi32 ch = read_ucs4be_char_from_file(str->file_pointer);
					if(ch == -1)
						return -1;
					str->read_count++;
					return (ch > 0xFF)? 0x3F : ch;
				}
				else /* Regular file */
				{
					int ch = fgetc(str->file_pointer);
					if(ch == EOF)
						return -1;
					
					str->read_count++;
					return ch;
				}
			}
			else /* Text mode is the same for Unicode and regular files */
			{
				glsi32 ch = read_utf8_char_from_file(str->file_pointer);
				if(ch == -1)
					return -1;
					
				str->read_count++;
				return (ch > 0xFF)? 0x3F : ch;
			}
		default:
			g_warning("%s: Reading from this kind of stream unsupported.",
				__func__);
			return -1;
	}
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
	g_return_val_if_fail(str != NULL, 0);
	g_return_val_if_fail(str->file_mode == filemode_Read
		|| str->file_mode == filemode_ReadWrite, 0);
	g_return_val_if_fail(buf != NULL, 0);
	
	switch(str->stream_type)
	{
		case STREAM_TYPE_MEMORY:
		{
			int copycount = 0;
			if(str->unicode)
			{
				while(copycount < len && str->ubuffer 
					&& str->mark < str->buflen) 
				{
					glui32 ch = str->ubuffer[str->mark++];
					buf[copycount++] = (ch > 0xFF)? '?' : (char)ch;
				}
			}
			else
			{
				if(str->buffer) /* if not, copycount stays 0 */
					copycount = min(len, str->buflen - str->mark);
				memmove(buf, str->buffer + str->mark, copycount);
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
					size_t count = fread(readbuffer, sizeof(unsigned char), 
						4 * len, str->file_pointer);
					/* If there was an incomplete character */
					if(count % 4 != 0) 
					{
						count -= count % 4;
						g_warning("%s: Incomplete character in binary Unicode "
							"file.", __func__);
					}
					
					str->read_count += count / 4;
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
					return count / 4;
				}
				else /* Regular binary file */
				{
					size_t count = fread(buf, sizeof(char), len, 
						str->file_pointer);
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
					glsi32 ch = read_utf8_char_from_file(str->file_pointer);
					if(ch == -1)
						break;
					str->read_count++;
					buf[foo] = (ch > 0xFF)? 0x3F : (gchar)ch;
				}
				return foo;
			}
		default:
			g_warning("%s: Reading from this kind of stream unsupported.",
				__func__);
			return 0;
	}
}

/**
 * glk_get_line_stream:
 * @str: An input stream.
 * @buf: A buffer with space for at least @len characters.
 * @len: The number of characters to read, plus one.
 *
 * Reads characters from @str, until either @len - 1 characters have been read
 * or a newline has been read. It then puts a terminal null ('\0') aracter on
 * the end. It returns the number of characters actually read, including the
 * newline (if there is one) but not including the terminal null.
 *
 * It is usually more efficient to read several characters at once with
 * glk_get_buffer_stream() or glk_get_line_stream(), as opposed to calling
 * glk_get_char_stream() several times.
 *
 * Returns: The number of characters actually read.
 */
glui32
glk_get_line_stream(strid_t str, char *buf, glui32 len)
{
	g_return_val_if_fail(str != NULL, 0);
	g_return_val_if_fail(str->file_mode == filemode_Read
		|| str->file_mode == filemode_ReadWrite, 0);
	g_return_val_if_fail(buf != NULL, 0);

	switch(str->stream_type)
	{
		case STREAM_TYPE_MEMORY:
		{
			int copycount = 0;
			if(str->unicode)
			{
				/* Do it character-by-character */
				while(copycount < len - 1 && str->ubuffer 
					&& str->mark < str->buflen) 
				{
					glui32 ch = str->ubuffer[str->mark++];
					/* Check for Unicode newline; slightly different than
					in file streams */
					if(ch == 0x0A || ch == 0x85 || ch == 0x0C || ch == 0x2028 
						|| ch == 0x2029)
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
					copycount = min(len, str->buflen - str->mark);
				memccpy(buf, str->buffer + str->mark, '\n', copycount);
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
					int foo;
					for(foo = 0; foo < len - 1; foo++)
					{
						glsi32 ch = 
							read_ucs4be_char_from_file(str->file_pointer);
						if(ch == -1) 
						{
							buf[foo] = '\0';
							return foo - 1;
						}
						str->read_count++;
						if(is_unicode_newline(ch, str->file_pointer, FALSE))
						{
							buf[foo] = '\n';
							buf[foo + 1] = '\0';
							return foo;
						}
						buf[foo] = (ch > 0xFF)? '?' : (char)ch;
					}
					buf[len] = '\0';
					return foo;
				}
				else /* Regular binary file */
				{
					fgets(buf, len, str->file_pointer);
					str->read_count += strlen(buf);
					return strlen(buf);
				}
			}
			else /* Text mode is the same for Unicode and regular files */
			{
				/* Do it character-by-character */
				int foo;
				for(foo = 0; foo < len - 1; foo++)
				{
					glsi32 ch = read_utf8_char_from_file(str->file_pointer);
					if(ch == -1)
					{
						buf[foo] = '\0';
						return foo - 1;
					}
					str->read_count++;
					if(is_unicode_newline(ch, str->file_pointer, TRUE))
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
			g_warning("%s: Reading from this kind of stream unsupported.",
				__func__);
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
 * Returns: position of the read/write mark in @str.
 */
glui32
glk_stream_get_position(strid_t str)
{
	g_return_val_if_fail(str != NULL, 0);
	
	switch(str->stream_type)
	{
		case STREAM_TYPE_MEMORY:
			return str->mark;
		case STREAM_TYPE_FILE:
			return ftell(str->file_pointer);
		default:
			g_warning("%s: Seeking not supported on this type of stream.",
				__func__);
			return 0;
	}
}

/**
 * glk_stream_set_position:
 * @str: A file or memory stream.
 * @pos: The position to set the mark to, relative to @seekmode.
 * @seekmode: One of #seekmode_Start, #seekmode_Current, or #seekmode_End.
 *
 * Sets the position of the read/write mark in @str. The position is controlled
 * by @pos, and the meaning of @pos is controlled by @seekmode:
 * <itemizedlist>
 *  <listitem>#seekmode_Start: @pos characters after the beginning of the file.
 *  </listitem>
 *  <listitem>#seekmode_Current: @pos characters after the current position
 *  (moving backwards if @pos is negative.)</listitem>
 *  <listitem>#seekmode_End: @pos characters after the end of the file. (@pos
 *  should always be zero or negative, so that this will move backwards to a
 *  position within the file.</listitem>
 * </itemizedlist>
 * It is illegal to specify a position before the beginning or after the end of
 * the file.
 *
 * In binary files, the mark position is exact --- it corresponds with the
 * number of characters you have read or written. In text files, this mapping 
 * can vary, because of linefeed conventions or other character-set 
 * approximations. glk_stream_set_position() and glk_stream_get_position()
 * measure positions in the platform's native encoding --- after character
 * cookery. Therefore, in a text stream, it is safest to use
 * glk_stream_set_position() only to move to the beginning or end of a file, or
 * to a position determined by glk_stream_get_position().
 *
 * Again, in Latin-1 streams, characters are bytes. In Unicode streams,
 * characters are 32-bit words, or four bytes each.
 */
void
glk_stream_set_position(strid_t str, glsi32 pos, glui32 seekmode)
{
	g_return_if_fail(str != NULL);
	g_return_if_fail(!(seekmode == seekmode_Start && pos < 0));
	g_return_if_fail(!(seekmode == seekmode_End || pos > 0));
	
	switch(str->stream_type)
	{
		case STREAM_TYPE_MEMORY:
			switch(seekmode)
			{
				case seekmode_Start:   str->mark = pos;  break;
				case seekmode_Current: str->mark += pos; break;
				case seekmode_End:     str->mark = str->buflen + pos; break;
				default:
					g_assert_not_reached();
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
					g_assert_not_reached();
					return;
			}
			fseek(str->file_pointer, pos, whence);
			break;
		}
		default:
			g_warning("%s: Seeking not supported on this type of stream.",
				__func__);
			return;
	}
}
 
