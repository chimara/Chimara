#include <glib.h>

#include "chimara-glk.h"
#include "magic.h"
#include "window.h"

/**
 * glk_request_mouse_event:
 * @win: Window on which to request a mouse input event.
 *
 * Requests mouse input on the window @win.
 */
void
glk_request_mouse_event(winid_t win)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win != NULL);
	g_return_if_fail(win->type == wintype_TextGrid || win->type == wintype_Graphics);

	g_signal_handler_unblock(win->widget, win->button_press_event_handler);
}

/**
 * glk_cancel_mouse_event:
 * @win: Window with a mouse input event pending.
 *
 * Cancels the pending mouse input request on @win.
 */
void 
glk_cancel_mouse_event(winid_t win)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win != NULL);
	g_return_if_fail(win->type == wintype_TextGrid || win->type == wintype_Graphics);

	g_signal_handler_block(win->widget, win->button_press_event_handler);
}
