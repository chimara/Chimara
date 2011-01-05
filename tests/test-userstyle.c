#include <libchimara/glk.h>

winid_t mainwin;

void 
glk_main(void)
{
	/* Create user style before creating windows */
	glk_stylehint_set(wintype_AllTypes, style_User1, stylehint_Size, -1);
	glk_stylehint_set(wintype_AllTypes, style_User2, stylehint_Size, 0);
	
	mainwin = glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	if(!mainwin)
		return;
	glk_set_window(mainwin);

	glk_set_style(style_User1);
	glk_put_string("This text is in User1 and slightly smaller");
	glk_set_style(style_Normal);
	glk_put_string(".\n");
	glk_set_style(style_User2);
	glk_put_string("This test is in User2 at normal size");
	glk_set_style(style_Normal);
	glk_put_string(".\n");
}
