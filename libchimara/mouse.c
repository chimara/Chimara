#include "mouse.h"
#include "magic.h"

void
glk_request_mouse_event(winid_t win)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win != NULL);
	g_return_if_fail(win->type == wintype_TextBuffer || win->type == wintype_TextGrid);

	g_signal_handler_unblock(win->widget, win->button_press_event_handler);
}

void 
glk_cancel_mouse_event(winid_t win)
{
	VALID_WINDOW(win, return);
	g_return_if_fail(win != NULL);
	g_return_if_fail(win->type == wintype_TextBuffer || win->type == wintype_TextGrid);

	g_signal_handler_block(win->widget, win->button_press_event_handler);
}

gboolean
on_window_button_press(GtkWidget *widget, GdkEventButton *event, winid_t win)
{
	ChimaraGlk *glk = CHIMARA_GLK(gtk_widget_get_ancestor(win->widget, CHIMARA_TYPE_GLK));
	g_assert(glk);

	/* TODO: calculate coordinates in proper metric */
	event_throw(glk, evtype_MouseInput, win, event->x, event->y);

	return TRUE;
}
