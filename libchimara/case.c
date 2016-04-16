#include <string.h>
#include <glib.h>
#include "glk.h"
#include "charset.h"

/**
 * glk_char_to_lower:
 * @ch: A Latin-1 character.
 *
 * You can convert Latin-1 characters between upper and lower case with two Glk
 * utility functions, glk_char_to_lower() and glk_char_to_upper(). These have a
 * few advantages over the standard ANSI `tolower()` and `toupper()` macros.
 * They work for the entire Latin-1 character set, including accented letters;
 * they behave consistently on all platforms, since they're part of the Glk
 * library; and they are safe for all characters.
 * That is, if you call glk_char_to_lower() on a lower-case character, or a
 * character which is not a letter, you'll get the argument back unchanged.
 *
 * The case-sensitive characters in Latin-1 are the ranges 0x41..0x5A,
 * 0xC0..0xD6, 0xD8..0xDE (upper case) and the ranges 0x61..0x7A, 0xE0..0xF6,
 * 0xF8..0xFE (lower case). These are arranged in parallel; so 
 * glk_char_to_lower() will add 0x20 to values in the upper-case ranges, and
 * glk_char_to_upper() will subtract 0x20 from values in the lower-case ranges. 
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
 * uppercase. Otherwise, leaves it unchanged. See glk_char_to_lower().
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
 * Unicode character conversion is trickier, and must be applied to character
 * arrays, not single characters. These functions 
 * (glk_buffer_to_lower_case_uni(), glk_buffer_to_upper_case_uni(), and
 * glk_buffer_to_title_case_uni()) provide two length arguments because a
 * string of Unicode characters may expand when its case changes. The @len
 * argument is the available length of the buffer; @numchars is the number of
 * characters in the buffer initially. (So @numchars must be less than or equal
 * to @len. The contents of the buffer after @numchars do not affect the
 * operation.)
 *
 * The functions return the number of characters after conversion. If this is
 * greater than @len, the characters in the array will be safely truncated at
 * @len, but the true count will be returned. (The contents of the buffer after
 * the returned count are undefined.)
 *
 * The `lower_case` and `upper_case` functions do what you'd expect: they
 * convert every character in the buffer (the first @numchars of them) to its
 * upper or lower-case equivalent, if there is such a thing.
 *
 * See the Unicode spec (chapter 3.13, chapter 4.2, etc) for the exact
 * definitions of upper, lower, and title-case mapping.
 * 
 * <note><para>
 *   Unicode has some strange case cases. For example, a combined character
 *   that looks like “ss” might properly be upper-cased into
 *   <emphasis>two</emphasis> “S” characters.
 *   Title-casing is even stranger; “ss” (at the beginning of a word) might be
 *   title-cased into a different combined character that looks like “Ss”.
 *   The glk_buffer_to_title_case_uni() function is actually title-casing the
 *   first character of the buffer.
 *   If it makes a difference.
 * </para></note>
 *
 * Returns: The number of characters after conversion.
 */
glui32
glk_buffer_to_lower_case_uni(glui32 *buf, glui32 len, glui32 numchars)
{
	g_return_val_if_fail(buf != NULL && (len > 0 || numchars > 0), 0);
	g_return_val_if_fail(numchars <= len, 0);

	long outchars;

	/* Lowercase the string */
	char *utf8 = convert_ucs4_to_utf8(buf, numchars);
	if(!utf8)
		return numchars;
	char *lowered = g_utf8_strdown(utf8, -1);
	g_free(utf8);
	gunichar *outbuf = convert_utf8_to_ucs4(lowered, &outchars);
	g_free(lowered);
	if(!outbuf)
		return numchars;
	
	/* Copy the output buffer to the original buffer */
	memcpy(buf, outbuf, MIN(outchars, len) * 4);
	g_free(outbuf);
	
	return outchars;
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
	
	long outchars;
	
	/* Uppercase the string */
	char *utf8 = convert_ucs4_to_utf8(buf, numchars);
	if(!utf8)
		return numchars;
	char *uppered = g_utf8_strup(utf8, -1);
	g_free(utf8);
	gunichar *outbuf = convert_utf8_to_ucs4(uppered, &outchars);
	g_free(uppered);
	if(!outbuf)
		return numchars;

	/* Copy the output buffer to the original buffer */
	memcpy(buf, outbuf, MIN(outchars, len) * 4);
	g_free(outbuf);

	return outchars;
}

/**
 * glk_buffer_to_title_case_uni:
 * @buf: A character array in UCS-4.
 * @len: Available length of @buf.
 * @numchars: Number of characters in @buf.
 * @lowerrest: %TRUE if the rest of @buf should be lowercased, %FALSE 
 * otherwise.
 *
 * See glk_buffer_to_lower_case_uni(). The `title_case` function has an
 * additional (boolean) flag. If the flag is zero, the function changes the
 * first character of the buffer to upper-case, and leaves the rest of the
 * buffer unchanged. If the flag is nonzero, it changes the first character to
 * upper-case and the rest to lower-case.
 *
 * <note><para>
 *   Earlier drafts of this spec had a separate function which title-cased the
 *   first character of every <emphasis>word</emphasis> in the buffer. I took
 *   this out after reading Unicode Standard Annex &num;29, which explains how
 *   to divide a string into words. If you want it, feel free to implement it.
 * </para></note>
 * 
 * Returns: The number of characters after conversion.
 */
glui32
glk_buffer_to_title_case_uni(glui32 *buf, glui32 len, glui32 numchars, glui32 lowerrest)
{
	g_return_val_if_fail(buf != NULL && (len > 0 || numchars > 0), 0);
	g_return_val_if_fail(numchars <= len, 0);
	
	/* FIXME: This is wrong. g_unichar_totitle() which returns the titlecase of
	 one Unicode code point, but that only returns the correct result if the
	 titlecase character is also one code point.
	 For example, the one-character 'ffi' ligature should be title-cased to the
	 three-character string 'Ffi'. This code leaves it as the 'ffi' ligature,
	 which is incorrect.
	 However, nothing much can be done about it unless GLib gets a
	 g_utf8_strtitle() function, or we roll our own. */
	*buf = g_unichar_totitle(*buf);
	/* Call lowercase on the rest of the string */
	if(lowerrest)
		return glk_buffer_to_lower_case_uni(buf + 1, len - 1, numchars - 1) + 1;
	return numchars;
}

/**
 * glk_buffer_canon_decompose_uni:
 * @buf: A character array in UCS-4.
 * @len: Available length of @buf.
 * @numchars: Number of characters in @buf.
 *
 * This transforms a string into its canonical decomposition (“Normalization
 * Form D”).
 * Effectively, this takes apart multipart characters into their individual
 * parts.
 * For example, it would convert “&egrave;” (character 0xE8, an accented “e”)
 * into the two-character string containing “e” followed by Unicode character
 * 0x0300 (COMBINING GRAVE ACCENT).
 * If a single character has multiple accent marks, they are also rearranged
 * into a standard order.
 *
 * Returns: The number of characters in @buf after decomposition.
 */
glui32
glk_buffer_canon_decompose_uni(glui32 *buf, glui32 len, glui32 numchars)
{
	g_return_val_if_fail(buf != NULL && (len > 0 || numchars > 0), 0);
	g_return_val_if_fail(numchars <= len, 0);

	long outchars;

	/* Normalize the string */
	char *utf8 = convert_ucs4_to_utf8(buf, numchars);
	if(!utf8)
		return numchars;
	char *decomposed = g_utf8_normalize(utf8, -1, G_NORMALIZE_NFD);
	g_free(utf8);
	gunichar *outbuf = convert_utf8_to_ucs4(decomposed, &outchars);
	g_free(decomposed);
	if(!outbuf)
		return numchars;

	/* Copy the output buffer to the original buffer */
	memcpy(buf, outbuf, MIN(outchars, len) * 4);
	g_free(outbuf);

	return outchars;
}

/**
 * glk_buffer_canon_normalize_uni:
 * @buf: A character array in UCS-4.
 * @len: Available length of @buf.
 * @numchars: Number of characters in @buf.
 *
 * This transforms a string into its canonical decomposition and recomposition
 * (“Normalization Form C”).
 * Effectively, this takes apart multipart characters, and then puts them back
 * together in a standard way.
 * For example, this would convert the two-character string containing “e”
 * followed by Unicode character 0x0300 (COMBINING GRAVE ACCENT) into the
 * one-character string “&egrave;” (character 0xE8, an accented “e”).
 *
 * The `canon_normalize` function includes decomposition as part of its
 * implementation.
 * You never have to call both functions on the same string.
 *
 * Both of these functions are idempotent.
 *
 * These functions provide two length arguments because a string of Unicode
 * characters may expand when it is transformed. The @len argument is the
 * available length of the buffer; @numchars is the number of characters in the
 * buffer initially. (So @numchars must be less than or equal to @len. The
 * contents of the buffer after @numchars do not affect the operation.)
 *
 * The functions return the number of characters after transformation. If this
 * is greater than @len, the characters in the array will be safely truncated at
 * @len, but the true count will be returned. (The contents of the buffer after
 * the returned count are undefined.)
 *
 * <note><para>
 *   The Unicode spec also defines stronger forms of these functions, called
 *   “compatibility decomposition and recomposition” (“Normalization Form KD”
 *   and “Normalization Form KC”.)
 *   These do all of the accent-mangling described above, but they also
 *   transform many other obscure Unicode characters into more familiar forms.
 *   For example, they split ligatures apart into separate letters.
 *   They also convert Unicode display variations such as script letters,
 *   circled letters, and half-width letters into their common forms.
 * </para></note>
 *
 * <note><para>
 *   The Glk spec does not currently provide these stronger transformations.
 *   Glk's expected use of Unicode normalization is for line input, and an OS
 *   facility for line input will generally not produce these alternate
 *   character forms (unless the user goes out of his way to type them).
 *   Therefore, the need for these transformations does not seem to be worth the
 *   extra data table space.
 * </para></note>
 *
 * Returns: the number of characters in @buf after normalization.
 */
glui32
glk_buffer_canon_normalize_uni(glui32 *buf, glui32 len, glui32 numchars)
{
	g_return_val_if_fail(buf != NULL && (len > 0 || numchars > 0), 0);
	g_return_val_if_fail(numchars <= len, 0);

	long outchars;

	/* Normalize the string */
	char *utf8 = convert_ucs4_to_utf8(buf, numchars);
	if(!utf8)
		return numchars;
	char *decomposed = g_utf8_normalize(utf8, -1, G_NORMALIZE_NFC);
	g_free(utf8);
	gunichar *outbuf = convert_utf8_to_ucs4(decomposed, &outchars);
	g_free(decomposed);
	if(!outbuf)
		return numchars;

	/* Copy the output buffer to the original buffer */
	memcpy(buf, outbuf, MIN(outchars, len) * 4);
	g_free(outbuf);

	return outchars;
}
