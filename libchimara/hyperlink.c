#include "hyperlink.h"
#include "chimara-glk-private.h"

extern GPrivate *glk_data_key;

/**
 * glk_set_hyperlink:
 * @linkval: Set to nonzero to initiate hyperlink mode. Set to zero to disengage.
 *
 * Use this function to create hyperlinks in a textbuffer. It sets the current stream
 * to hyperlink mode, after which text will become a hyperlink until hyperlink mode
 * is turned off. If the current stream does not write to a textbuffer window, this function
 * does nothing.
 *
 * You can request hyperlink events with glk_request_hyperlink_event() to react
 * to clicks on the link.
 */
void 
glk_set_hyperlink(glui32 linkval)
{
	ChimaraGlkPrivate *glk_data = g_private_get(glk_data_key);
	g_return_if_fail(glk_data->current_stream != NULL);
	glk_set_hyperlink_stream(glk_data->current_stream, linkval);
}

/**
 * glk_set_hyperlink:
 * @str: The stream to set the hyperlink mode on.
 * @linkval: Set to nonzero to initiate hyperlink mode. Set to zero to disengage.
 *
 * Use this function to create hyperlinks in a textbuffer. It sets a stream to a textbuffer
 * window to hyperlink mode, after which text will become a hyperlink until hyperlink mode
 * is turned off. Calling this function on a stream that does not write to a textbuffer does
 * nothing.
 *
 * You can request hyperlink events with glk_request_hyperlink_event() to react
 * to clicks on the link.
 */
void 
glk_set_hyperlink_stream(strid_t str, glui32 linkval)
{
	g_return_if_fail(str != NULL);
	g_return_if_fail(str->window != NULL);
	g_return_if_fail(str->window->type == wintype_TextBuffer);

	str->hyperlink_mode = (linkval != 0);
}

void 
glk_request_hyperlink_event(winid_t win)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win != NULL);
	g_return_if_fail(win->mouse_click_handler != 0);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);

	g_signal_handler_unblock( G_OBJECT(win->widget), win->mouse_click_handler );
}

void 
glk_cancel_hyperlink_event(winid_t win)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win != NULL);
	g_return_if_fail(win->mouse_click_handler != 0);
	g_return_if_fail(win->type != wintype_TextBuffer || win->type != wintype_TextGrid);
	
	g_signal_handler_block( G_OBJECT(win->widget), win->mouse_click_handler );
}

/* Internal function: General callback for signal button-release-event on a
 * text buffer or text grid window.  Used for detecting clicks on hyperlinks.
 * Blocked when not in use.
 */
gboolean
on_window_button_release_event(GtkWidget *widget, GdkEventButton *event, winid_t win)
{
	printf("Click on (%f,%f)\n", event->x, event->y);
	return TRUE;
}
