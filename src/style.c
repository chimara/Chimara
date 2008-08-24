#include "glk.h"

/**
 * glk_set_style:
 * @val: A style.
 *
 * Changes the style of the current output stream. @val should be one of 
 * #style_Normal, #style_Emphasized, #style_Preformatted, #style_Header,
 * #style_Subheader, #style_Alert, #style_Note, #style_BlockQuote, #style_Input,
 * #style_User1, or #style_User2. However, any value is actually legal; if the
 * library does not recognize the style value, it will treat it as
 * #style_Normal. (This policy allows for the future definition of styles
 * without breaking old Glk libraries.) 
 */
void
glk_set_style(glui32 val)
{
	/* No nothing yet */
	return;
}
