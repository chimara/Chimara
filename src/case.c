#include <gtk/gtk.h>
#include "glk.h"

/**
 * glk_char_to_lower:
 * @ch: A Latin-1 character.
 *
 * If @ch is an uppercase character in the Latin-1 character set, converts it
 * to lowercase. Otherwise, leaves it unchanged.
 *
 * Returns: A lowercase or non-letter Latin-1 character.
 */
unsigned char
glk_char_to_lower(unsigned char ch)
{
	if( (ch >= 0x41 && ch <= 0x5A) || (ch >= 0xC0 && ch <= 0xD6) || (ch >= 0xD8 && ch <= 0xDE) )
		return ch + 0x20;
	return ch;
}

/**
 * glk_char_to_upper:
 * @ch: A Latin-1 character.
 *
 * If @ch is a lowercase character in the Latin-1 character set, converts it to
 * uppercase. Otherwise, leaves it unchanged.
 *
 * Returns: An uppercase or non-letter Latin-1 character.
 */
unsigned char
glk_char_to_upper(unsigned char ch)
{
	if( (ch >= 0x61 && ch <= 0x7A) || (ch >= 0xE0 && ch <= 0xF6) || (ch >= 0xF8 && ch <= 0xFE) )
		return ch - 0x20;
	return ch;
}

/**
 * glk_buffer_to_lower_case_uni:
 * @buf: A character array in UCS-4.
 * @len: Available length of @buf.
 * @numchars: Number of characters in @buf.
 *
 * Converts the first @numchars characters of @buf to their lowercase 
 * equivalents, if there is such a thing. These functions provide two length
 * arguments because a string of Unicode characters may expand when its case
 * changes. The @len argument is the available length of the buffer; @numchars
 * is the number of characters in the buffer initially. (So @numchars must be
 * less than or equal to @len. The contents of the buffer after @numchars do
 * not affect the operation.)
 *
 * Returns: The number of characters after conversion. If this is greater than
 * @len, the characters in the array will be safely truncated at len, but the
 * true count will be returned. (The contents of the buffer after the returned
 * count are undefined.)
 */
glui32
glk_buffer_to_lower_case_uni(glui32 *buf, glui32 len, glui32 numchars)
{
	g_return_val_if_fail(buf != NULL && (len > 0 || numchars > 0), 0);
	g_return_val_if_fail(numchars <= len, 0);
	
	/* GLib has a function that converts _one_ UCS-4 character to _one_
	lowercase UCS-4 character; so apparently we don't have to worry about the
	string length changing... */
	glui32 *ptr;
	for(ptr = buf; ptr < buf + numchars; ptr++)
		*ptr = g_unichar_tolower(*ptr);
	
	return numchars;
}

/**
 * glk_buffer_to_upper_case_uni:
 * @buf: A character array in UCS-4.
 * @len: Available length of @buf.
 * @numchars: Number of characters in @buf.
 *
 * Converts the first @numchars characters of @buf to their uppercase 
 * equivalents, if there is such a thing. See glk_buffer_to_lower_case_uni().
 *
 * Returns: The number of characters after conversion.
 */
glui32
glk_buffer_to_upper_case_uni(glui32 *buf, glui32 len, glui32 numchars)
{
	g_return_val_if_fail(buf != NULL && (len > 0 || numchars > 0), 0);
	g_return_val_if_fail(numchars <= len, 0);
	
	/* GLib has a function that converts _one_ UCS-4 character to _one_
	uppercase UCS-4 character; so apparently we don't have to worry about the
	string length changing... */
	glui32 *ptr;
	for(ptr = buf; ptr < buf + numchars; ptr++)
		*ptr = g_unichar_toupper(*ptr);
	
	return numchars;
}

/**
 * glk_buffer_to_title_case_uni:
 * @buf: A character array in UCS-4.
 * @len: Available length of @buf.
 * @numchars: Number of characters in @buf.
 * @lowerrest: %TRUE if the rest of @buf should be lowercased, %FALSE 
 * otherwise.
 *
 * Converts the first character of @buf to uppercase, if there is such a thing.
 * See glk_buffer_to_lower_case_uni(). If @lowerrest is %TRUE, then the
 * remainder of @buf is lowercased.
 *
 * Returns: The number of characters after conversion.
 */
glui32
glk_buffer_to_title_case_uni(glui32 *buf, glui32 len, glui32 numchars, glui32 lowerrest)
{
	g_return_val_if_fail(buf != NULL && (len > 0 || numchars > 0), 0);
	g_return_val_if_fail(numchars <= len, 0);
	
	/* GLib has a function that converts _one_ UCS-4 character to _one_
	titlecase UCS-4 character; so apparently we don't have to worry about the
	string length changing... */
	*buf = g_unichar_totitle(*buf);
	/* Call lowercase on the rest of the string */
	if(lowerrest)
		return glk_buffer_to_lower_case_uni(buf + 1, len - 1, numchars - 1) + 1;
	return numchars;
}

