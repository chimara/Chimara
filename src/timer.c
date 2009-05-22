#include "timer.h"

extern ChimaraGlkPrivate *glk_data;

/**
 * You can request that an event be sent at fixed intervals, regardless of what
 * the player does. Unlike input events, timer events can be tested for with
 * glk_select_poll() as well as glk_select(). 
 *
 * Initially, there is no timer and you get no timer events. If you call
 * glk_request_timer_events(N), with N not 0, you will get timer events about
 * every N milliseconds thereafter. (Assuming that they are supported -- if
 * not, glk_request_timer_events() has no effect.) Unlike keyboard and mouse
 * events, timer events will continue until you shut them off. You do not have
 * to re-request them every time you get one. Call glk_request_timer_events(0)
 * to stop getting timer events. 
 *
 * The rule is that when you call glk_select() or glk_select_poll(), if it has
 * been more than N milliseconds since the last timer event, and (for
 * glk_select()) if there is no player input, you will receive an event whose
 * type is evtype_Timer. (win, val1, and val2 will all be 0.) 
 *
 * Timer events do not stack up. If you spend 10N milliseconds doing
 * computation, and then call glk_select(), you will not get ten timer events
 * in a row. The library will simply note that it has been more than N
 * milliseconds, and return a timer event right away. If you call glk_select()
 * again immediately, it will be N milliseconds before the next timer event. 
 *
 * This means that the timing of timer events is approximate, and the library
 * will err on the side of being late. If there is a conflict between player
 * input events and timer events, the player input takes precedence. [This
 * prevents the user from being locked out by overly enthusiastic timer events.
 * Unfortunately, it also means that your timer can be locked out on slower
 * machines, if the player pounds too enthusiastically on the keyboard. Sorry.
 * If you want a real-time operating system, talk to Wind River.] 
 *
 * [I don't have to tell you that a millisecond is one thousandth of a second,
 * do I?]
 *
 * NOTE: setting a new timer will overwrite the old timer if one was in place.
 */
void
glk_request_timer_events(glui32 millisecs)
{
	// Stop any existing timer
	if(glk_data->timer_id != 0) {
		g_source_remove(glk_data->timer_id);
		glk_data->timer_id = 0;
	}

	if(millisecs == 0)
		return;
	
	glk_data->timer_id = g_timeout_add(millisecs, push_timer_event, NULL);
}

/**
 * Internal function: push a new timer event on the event stack.
 * Will always return TRUE
 */
gboolean
push_timer_event(gpointer data)
{
	event_throw(evtype_Timer, NULL, 0, 0);

	return TRUE;
}
