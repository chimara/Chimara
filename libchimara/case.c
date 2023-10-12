#include <string.h>

#include <glib.h>

#include "charset.h"
#include "glk.h"

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

	/* Special-cases not handled by g_unichar_totitle() because they expand to
	 * more than one character in title case.
	 * Source: https://www.unicode.org/Public/UCD/latest/ucd/SpecialCasing.txt
	 * Does not include any context- or locale-sensitive case mappings. */
	unsigned tlen = 0;
	glui32 titled[3];
	switch (buf[0]) {
	case 0x00df:  /* LATIN SMALL LETTER SHARP S */
		titled[tlen++] = 0x0053;
		titled[tlen++] = 0x0073;
		break;
	case 0xfb00:  /* LATIN SMALL LIGATURE FF */
		titled[tlen++] = 0x0046;
		titled[tlen++] = 0x0066;
		break;
	case 0xfb01:  /* LATIN SMALL LIGATURE FI */
		titled[tlen++] = 0x0046;
		titled[tlen++] = 0x0069;
		break;
	case 0xfb02:  /* LATIN SMALL LIGATURE FL */
		titled[tlen++] = 0x0046;
		titled[tlen++] = 0x006c;
		break;
	case 0xfb03:  /* LATIN SMALL LIGATURE FFI */
		titled[tlen++] = 0x0046;
		titled[tlen++] = 0x0066;
		titled[tlen++] = 0x0069;
		break;
	case 0xfb04:  /* LATIN SMALL LIGATURE FFL */
		titled[tlen++] = 0x0046;
		titled[tlen++] = 0x0066;
		titled[tlen++] = 0x006c;
		break;
	case 0xfb05:  /* LATIN SMALL LIGATURE LONG S T */
		titled[tlen++] = 0x0053;
		titled[tlen++] = 0x0074;
		break;
	case 0xfb06:  /* LATIN SMALL LIGATURE ST */
		titled[tlen++] = 0x0053;
		titled[tlen++] = 0x0074;
		break;
	case 0x0587:  /* ARMENIAN SMALL LIGATURE ECH YIWN */
		titled[tlen++] = 0x0535;
		titled[tlen++] = 0x0582;
		break;
	case 0xfb13:  /* ARMENIAN SMALL LIGATURE MEN NOW */
		titled[tlen++] = 0x0544;
		titled[tlen++] = 0x0576;
		break;
	case 0xfb14:  /* ARMENIAN SMALL LIGATURE MEN ECH */
		titled[tlen++] = 0x0544;
		titled[tlen++] = 0x0565;
		break;
	case 0xfb15:  /* ARMENIAN SMALL LIGATURE MEN INI */
		titled[tlen++] = 0x0544;
		titled[tlen++] = 0x056b;
		break;
	case 0xfb16:  /* ARMENIAN SMALL LIGATURE VEW NOW */
		titled[tlen++] = 0x054e;
		titled[tlen++] = 0x0576;
		break;
	case 0xfb17:  /* ARMENIAN SMALL LIGATURE MEN XEH */
		titled[tlen++] = 0x0544;
		titled[tlen++] = 0x056d;
		break;
	case 0x0149:  /* LATIN SMALL LETTER N PRECEDED BY APOSTROPHE */
		titled[tlen++] = 0x02bc;
		titled[tlen++] = 0x004e;
		break;
	case 0x0390:  /* GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TONOS */
		titled[tlen++] = 0x0399;
		titled[tlen++] = 0x0308;
		titled[tlen++] = 0x0301;
		break;
	case 0x03b0:  /* GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND TONOS */
		titled[tlen++] = 0x03a5;
		titled[tlen++] = 0x0308;
		titled[tlen++] = 0x0301;
		break;
	case 0x01f0:  /* LATIN SMALL LETTER J WITH CARON */
		titled[tlen++] = 0x004a;
		titled[tlen++] = 0x030c;
		break;
	case 0x1e96:  /* LATIN SMALL LETTER H WITH LINE BELOW */
		titled[tlen++] = 0x0048;
		titled[tlen++] = 0x0331;
		break;
	case 0x1e97:  /* LATIN SMALL LETTER T WITH DIAERESIS */
		titled[tlen++] = 0x0054;
		titled[tlen++] = 0x0308;
		break;
	case 0x1e98:  /* LATIN SMALL LETTER W WITH RING ABOVE */
		titled[tlen++] = 0x0057;
		titled[tlen++] = 0x030a;
		break;
	case 0x1e99:  /* LATIN SMALL LETTER Y WITH RING ABOVE */
		titled[tlen++] = 0x0059;
		titled[tlen++] = 0x030a;
		break;
	case 0x1e9a:  /* LATIN SMALL LETTER A WITH RIGHT HALF RING */
		titled[tlen++] = 0x0041;
		titled[tlen++] = 0x02be;
		break;
	case 0x1f50:  /* GREEK SMALL LETTER UPSILON WITH PSILI */
		titled[tlen++] = 0x03a5;
		titled[tlen++] = 0x0313;
		break;
	case 0x1f52:  /* GREEK SMALL LETTER UPSILON WITH PSILI AND VARIA */
		titled[tlen++] = 0x03a5;
		titled[tlen++] = 0x0313;
		titled[tlen++] = 0x0300;
		break;
	case 0x1f54:  /* GREEK SMALL LETTER UPSILON WITH PSILI AND OXIA */
		titled[tlen++] = 0x03a5;
		titled[tlen++] = 0x0313;
		titled[tlen++] = 0x0301;
		break;
	case 0x1f56:  /* GREEK SMALL LETTER UPSILON WITH PSILI AND PERISPOMENI */
		titled[tlen++] = 0x03a5;
		titled[tlen++] = 0x0313;
		titled[tlen++] = 0x0342;
		break;
	case 0x1fb6:  /* GREEK SMALL LETTER ALPHA WITH PERISPOMENI */
		titled[tlen++] = 0x0391;
		titled[tlen++] = 0x0342;
		break;
	case 0x1fc6:  /* GREEK SMALL LETTER ETA WITH PERISPOMENI */
		titled[tlen++] = 0x0397;
		titled[tlen++] = 0x0342;
		break;
	case 0x1fd2:  /* GREEK SMALL LETTER IOTA WITH DIALYTIKA AND VARIA */
		titled[tlen++] = 0x0399;
		titled[tlen++] = 0x0308;
		titled[tlen++] = 0x0300;
		break;
	case 0x1fd3:  /* GREEK SMALL LETTER IOTA WITH DIALYTIKA AND OXIA */
		titled[tlen++] = 0x0399;
		titled[tlen++] = 0x0308;
		titled[tlen++] = 0x0301;
		break;
	case 0x1fd6:  /* GREEK SMALL LETTER IOTA WITH PERISPOMENI */
		titled[tlen++] = 0x0399;
		titled[tlen++] = 0x0342;
		break;
	case 0x1fd7:  /* GREEK SMALL LETTER IOTA WITH DIALYTIKA AND PERISPOMENI */
		titled[tlen++] = 0x0399;
		titled[tlen++] = 0x0308;
		titled[tlen++] = 0x0342;
		break;
	case 0x1fe2:  /* GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND VARIA */
		titled[tlen++] = 0x03a5;
		titled[tlen++] = 0x0308;
		titled[tlen++] = 0x0300;
		break;
	case 0x1fe3:  /* GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND OXIA */
		titled[tlen++] = 0x03a5;
		titled[tlen++] = 0x0308;
		titled[tlen++] = 0x0301;
		break;
	case 0x1fe4:  /* GREEK SMALL LETTER RHO WITH PSILI */
		titled[tlen++] = 0x03a1;
		titled[tlen++] = 0x0313;
		break;
	case 0x1fe6:  /* GREEK SMALL LETTER UPSILON WITH PERISPOMENI */
		titled[tlen++] = 0x03a5;
		titled[tlen++] = 0x0342;
		break;
	case 0x1fe7:  /* GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND PERISPOMENI */
		titled[tlen++] = 0x03a5;
		titled[tlen++] = 0x0308;
		titled[tlen++] = 0x0342;
		break;
	case 0x1ff6:  /* GREEK SMALL LETTER OMEGA WITH PERISPOMENI */
		titled[tlen++] = 0x03a9;
		titled[tlen++] = 0x0342;
		break;
	case 0x1fb2:  /* GREEK SMALL LETTER ALPHA WITH VARIA AND YPOGEGRAMMENI */
		titled[tlen++] = 0x1fba;
		titled[tlen++] = 0x0345;
		break;
	case 0x1fb4:  /* GREEK SMALL LETTER ALPHA WITH OXIA AND YPOGEGRAMMENI */
		titled[tlen++] = 0x0386;
		titled[tlen++] = 0x0345;
		break;
	case 0x1fc2:  /* GREEK SMALL LETTER ETA WITH VARIA AND YPOGEGRAMMENI */
		titled[tlen++] = 0x1fca;
		titled[tlen++] = 0x0345;
		break;
	case 0x1fc4:  /* GREEK SMALL LETTER ETA WITH OXIA AND YPOGEGRAMMENI */
		titled[tlen++] = 0x0389;
		titled[tlen++] = 0x0345;
		break;
	case 0x1ff2:  /* GREEK SMALL LETTER OMEGA WITH VARIA AND YPOGEGRAMMENI */
		titled[tlen++] = 0x1ffa;
		titled[tlen++] = 0x0345;
		break;
	case 0x1ff4:  /* GREEK SMALL LETTER OMEGA WITH OXIA AND YPOGEGRAMMENI */
		titled[tlen++] = 0x038f;
		titled[tlen++] = 0x0345;
		break;
	case 0x1fb7:  /* GREEK SMALL LETTER ALPHA WITH PERISPOMENI AND YPOGEGRAMMENI */
		titled[tlen++] = 0x0391;
		titled[tlen++] = 0x0342;
		titled[tlen++] = 0x0345;
		break;
	case 0x1fc7:  /* GREEK SMALL LETTER ETA WITH PERISPOMENI AND YPOGEGRAMMENI */
		titled[tlen++] = 0x0397;
		titled[tlen++] = 0x0342;
		titled[tlen++] = 0x0345;
		break;
	case 0x1ff7:  /* GREEK SMALL LETTER OMEGA WITH PERISPOMENI AND YPOGEGRAMMENI */
		titled[tlen++] = 0x03a9;
		titled[tlen++] = 0x0342;
		titled[tlen++] = 0x0345;
		break;
	}

	if (tlen == 0) {
		/* Easy path. g_unichar_totitle() returns the titlecase of one Unicode
		 * code point, but that is only correct if the titlecase character is
		 * also one code point. */
		*buf = g_unichar_totitle(*buf);
		/* Call lowercase on the rest of the string */
		if (lowerrest)
			return glk_buffer_to_lower_case_uni(buf + 1, len - 1, numchars - 1) + 1;
		return numchars;
	}

	/* This code handles the special cases from the above switch statement. For
	 * example, the one-character 'ffi' ligature should be title-cased to the
	 * three-character string 'Ffi'. */

	/* Allocate a new buffer, because the number of characters has already
	 * changed. */
	g_autofree glui32 *newbuf = g_new0(glui32, len + tlen);

	memcpy(newbuf, titled, tlen * sizeof(glui32));
	memcpy(newbuf + tlen, buf + 1, (numchars - 1) * sizeof(glui32));

	if (!lowerrest) {
		memcpy(buf, newbuf, len * sizeof(glui32));
		return numchars + tlen - 1;
	}

	glui32 lowercount = glk_buffer_to_lower_case_uni(newbuf + tlen, len - tlen, numchars - 1);
	memcpy(buf, newbuf, len * sizeof(glui32));
	return lowercount + tlen;
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
