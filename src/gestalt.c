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
 * @arr: Location of an array to store extra information in, or #NULL.
 * @arrlen: Length of @arr, or 0 if @arr is #NULL.
 *
 * Calls the gestalt system to request information about selector @sel,
 * possibly returning information in @arr.
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
			return MAJOR_VERSION << 16 + MINOR_VERSION << 8 + SUB_VERSION;
		
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
			
		/* Selector not supported */	
		default:
			return 0;
	}
}

