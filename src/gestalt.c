#include "glk.h"

/* Version of the Glk specification implemented by this library */
#define MAJOR_VERSION 0
#define MINOR_VERSION 7
#define SUB_VERSION   0

/**
 * glk_gestalt:
 * @sel: A selector, representing which capability to request information 
 * about.
 * @val: Extra information, depending on the value of @sel.
 *
 * Calls the gestalt system to request information about selector @sel, without
 * passing an array to store extra information in (see glk_gestalt_ext()).
 *
 * Returns: an integer, depending on what selector was called.
 */
glui32
glk_gestalt(glui32 sel, glui32 val)
{
	return glk_gestalt_ext(sel, val, NULL, 0);
}

/**
 * glk_gestalt_ext:
 * @sel: A selector, representing which capability to request information
 * about.
 * @val: Extra information, depending on the value of @sel.
 * @arr: Location of an array to store extra information in, or %NULL.
 * @arrlen: Length of @arr, or 0 if @arr is %NULL.
 *
 * Calls the gestalt system to request information about the capabilities of the
 * API. The selector @sel tells which capability you are requesting information
 * about; the other three arguments are additional information, which may or may
 * not be meaningful. The @arr and @arrlen arguments are always optional; you
 * may always pass %NULL and 0, if you do not want whatever information they
 * represent. glk_gestalt() is simply a shortcut for this; glk_gestalt(x, y) is
 * exactly the same as glk_gestalt_ext(x, y, NULL, 0).
 *
 * The critical point is that if the Glk library has never heard of the selector
 * sel, it will return 0. It is always safe to call glk_gestalt(x, y) (or
 * glk_gestalt_ext(x, y, NULL, 0)). Even if you are using an old library, which
 * was compiled before the given capability was imagined, you can test for the
 * capability by calling glk_gestalt(); the library will correctly indicate that
 * it does not support it, by returning 0.
 *
 * <note><para>
 *  It is also safe to call glk_gestalt_ext(x, y, z, zlen) for an unknown 
 *  selector x, where z is not %NULL, as long as z points at an array of at 
 *  least zlen elements. The selector will be careful not to write beyond that  
 *  point in the array, if it writes to the array at all.
 * </para></note>
 *
 * <note><para>
 *  If a selector does not use the second argument, you should always pass 0; do
 *  not assume that the second argument is simply ignored. This is because the
 *  selector may be extended in the future. You will continue to get the current
 *  behavior if you pass 0 as the second argument, but other values may produce
 *  other behavior.
 * </para></note>
 *
 * Returns: an integer, depending on what selector was called.
 */
glui32
glk_gestalt_ext(glui32 sel, glui32 val, glui32 *arr, glui32 arrlen)
{
	switch(sel)
	{
		/* Version number */
		case gestalt_Version:
			return (MAJOR_VERSION << 16) + (MINOR_VERSION << 8) + SUB_VERSION;
		
		/* Which characters can we print? */	
		case gestalt_CharOutput:
			/* All characters are printed as one character, in any case */
			if(arr && arrlen > 0)
				*arr = 1;
			/* Cannot print control chars except \n, or chars > 255 */
			if( (val < 32 && val != 10) || (val >= 127 && val <= 159) || (val > 255) )
				return gestalt_CharOutput_CannotPrint;
			/* Can print all other Latin-1 characters */
			return gestalt_CharOutput_ExactPrint;
		
		/* Which characters can the player type in line input? */
		case gestalt_LineInput:
			/* Does not accept control chars */
			if( val < 32 || (val >= 127 && val <= 159) )
				return 0;
			return 1;
			
		/* Which characters can the player type in char input? */
		case gestalt_CharInput:
			/* Does not accept control chars or unknown */
			if( val < 32 || (val >= 127 && val <= 159) || val == keycode_Unknown )
				return 0;
			return 1;
			
		/* Selector not supported */	
		default:
			return 0;
	}
}

